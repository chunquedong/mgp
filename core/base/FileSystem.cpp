
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
    #include <windows.h>
    #include <tchar.h>
    #include <stdio.h>
    #include <direct.h>
    #define gp_stat _stat
    #define gp_stat_struct struct stat
#else
    #define __EXT_POSIX2
    #include <libgen.h>
    #include <dirent.h>
    #define gp_stat stat
    #define gp_stat_struct struct stat
#endif

#ifdef __ANDROID__
    #include <android/asset_manager.h>
    extern AAssetManager* __assetManager;
#endif

#include "base/Base.h"
#include "FileSystem.h"
#include "base/Properties.h"
#include "Stream.h"
#include "FileStream.h"

#include <algorithm>
#include <assert.h>
#include <mutex>

namespace mgp
{

#ifdef __ANDROID__
#include <unistd.h>

/**
 * Returns true if the file exists in the android read-only asset directory.
 */
static bool androidFileExists(const char* filePath)
{
    AAsset* asset = AAssetManager_open(__assetManager, filePath, AASSET_MODE_RANDOM);
    if (asset)
    {
        int lenght = AAsset_getLength(asset);
        AAsset_close(asset);
        return length > 0;
    }
    return false;
}

#endif

/** @script{ignore} */
static std::string __resourcePath("./");
static std::string __assetPath("");
static std::map<std::string, std::string> __aliases;
static std::mutex __fileLock;

/**
 * Gets the fully resolved path.
 * If the path is relative then it will be prefixed with the resource path.
 * Aliases will be converted to a relative path.
 * 
 * @param path The path to resolve.
 * @param fullPath The full resolved path. (out param)
 */
void getFullPath(const char* path, std::string& fullPath)
{
    if (FileSystem::isAbsolutePath(path))
    {
        fullPath.assign(path);
    }
    else
    {
        fullPath.assign(__resourcePath);
        fullPath += FileSystem::resolvePath(path);
    }
}

/////////////////////////////

FileSystem::FileSystem()
{
}

FileSystem::~FileSystem()
{
}

void FileSystem::setResourcePath(const char* path)
{
    __resourcePath = path == NULL ? "" : path;
}

const char* FileSystem::getResourcePath()
{
    return __resourcePath.c_str();
}

void FileSystem::loadResourceAliases(const char* aliasFilePath)
{
    UPtr<Properties> properties = Properties::create(aliasFilePath);
    if (properties.get())
    {
        Properties* aliases;
        while ((aliases = properties->getNextNamespace()) != NULL)
        {
            loadResourceAliases(aliases);
        }
    }
    //SAFE_DELETE(properties);
}

void FileSystem::loadResourceAliases(Properties* properties)
{
    assert(properties);

    const char* name;
    while ((name = properties->getNextProperty()) != NULL)
    {
        __aliases[name] = properties->getString();
    }
}

bool FileSystem::mkdirs(const char* cpath) {
    //printf("mkdirs: %s\n", cpath);
    std::lock_guard<std::mutex> guard(__fileLock);

    std::vector<std::string> dirs;
    std::string path = cpath;
    std::string dirPath;
    while (path.length() > 0)
    {
        int index = path.find('/');
        if (index == -1) {
            dirs.push_back(path);
        }
        else if (index == 0) {
            dirPath = "/";
        }
        else {
            dirs.push_back(path.substr(0, index));
        }

        if (index + 1 >= path.length() || index == -1)
            break;

        path = path.substr(index + 1);
    }

    for (unsigned int i = 0; i < dirs.size(); i++)
    {
        if (i > 0) dirPath += "/";
        dirPath += dirs[i];
        //printf("mkdir: %s\n", dirPath.c_str());
#ifdef _WIN32
        DWORD rc = GetFileAttributesA(dirPath.c_str());
        if (rc == INVALID_FILE_ATTRIBUTES) {
            if (CreateDirectoryA(dirPath.c_str(), NULL) == 0) {
                GP_ERROR("Failed to create directory: '%s'", dirPath.c_str());
                return false;
            }
        }
#else
        struct stat s;
        if (stat(dirPath.c_str(), &s) != 0)
        {
            // Directory does not exist.
            if (mkdir(dirPath.c_str(), 0777) != 0)
            {
                GP_ERROR("Failed to create directory: '%s'", dirPath.c_str());
                return false;
            }
        }
#endif
    }
    return false;
}

const char* FileSystem::resolvePath(const char* path)
{
    GP_ASSERT(path);

    size_t len = strlen(path);
    if (len > 1 && path[0] == '@')
    {
        std::string alias(path + 1);
        std::map<std::string, std::string>::const_iterator itr = __aliases.find(alias);
        if (itr == __aliases.end())
            return path; // no matching alias found
        return itr->second.c_str();
    }

    return path;
}

bool FileSystem::listFiles(const char* dirPath, std::vector<std::string>& files)
{
#ifdef _WIN32
    std::string path(FileSystem::getResourcePath());
    if (dirPath && strlen(dirPath) > 0)
    {
        path.append(dirPath);
    }
    path.append("/*");
    // Convert char to wchar
    std::basic_string<TCHAR> wPath;
    wPath.assign(path.begin(), path.end());

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(wPath.c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) 
    {
        return false;
    }
    do
    {
        // Add to the list if this is not a directory
        if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            // Convert wchar to char
            std::basic_string<TCHAR> wfilename(FindFileData.cFileName);
            std::string filename;
            filename.assign(wfilename.begin(), wfilename.end());
            files.push_back(filename);
        }
    } while (FindNextFile(hFind, &FindFileData) != 0);

    FindClose(hFind);
    return true;
#else
    std::string path(FileSystem::getResourcePath());
    if (dirPath && strlen(dirPath) > 0)
    {
        path.append(dirPath);
    }
    path.append("/.");
    bool result = false;

    struct dirent* dp;
    DIR* dir = opendir(path.c_str());
    if (dir != NULL)
    {
        while ((dp = readdir(dir)) != NULL)
        {
            std::string filepath(path);
            filepath.append("/");
            filepath.append(dp->d_name);

            struct stat buf;
            if (!stat(filepath.c_str(), &buf))
            {
                // Add to the list if this is not a directory
                if (!S_ISDIR(buf.st_mode))
                {
                    files.push_back(dp->d_name);
                }
            }
        }
        closedir(dir);
        result = true;
    }

#ifdef __ANDROID__
    // List the files that are in the android APK at this path
    AAssetDir* assetDir = AAssetManager_openDir(__assetManager, dirPath);
    if (assetDir != NULL)
    {
        AAssetDir_rewind(assetDir);
        const char* file = NULL;
        while ((file = AAssetDir_getNextFileName(assetDir)) != NULL)
        {
            std::string filename(file);
            // Check if this file was already added to the list because it was copied to the SD card.
            if (find(files.begin(), files.end(), filename) == files.end())
            {
                files.push_back(filename);
            }
        }
        AAssetDir_close(assetDir);
        result = true;
    }
#endif

    return result;
#endif
}

bool FileSystem::fileExists(const char* filePath)
{
    GP_ASSERT(filePath);

    std::string fullPath;

#ifdef __ANDROID__
    fullPath = __assetPath;
    fullPath += resolvePath(filePath);

    if (androidFileExists(fullPath.c_str()))
    {
        return true;
    }
#endif

    getFullPath(filePath, fullPath);

    gp_stat_struct s;
    return stat(fullPath.c_str(), &s) == 0;

}

UPtr<Stream> FileSystem::open(const char* path, size_t streamMode)
{
    char modeStr[] = "rb";
    if ((streamMode & WRITE) != 0)
        modeStr[0] = 'w';
#ifdef __ANDROID__
    std::string fullPath(__resourcePath);
    fullPath += resolvePath(path);

    if ((streamMode & WRITE) != 0)
    {
        // Open a file on the SD card
        size_t index = fullPath.rfind('/');
        if (index != std::string::npos)
        {
            std::string directoryPath = fullPath.substr(0, index);
            gp_stat_struct s;
            if (stat(directoryPath.c_str(), &s) != 0)
                mkdirs(directoryPath);
        }
        return FileStream::create(fullPath.c_str(), modeStr);
    }
    else
    {
        // First try the SD card
        UPtr<Stream> stream = FileStream::create(fullPath.c_str(), modeStr).dynamicCastTo<Stream>();

        if (!stream)
        {
            // Otherwise fall-back to assets loaded via the AssetManager
            fullPath = __assetPath;
            fullPath += resolvePath(path);

            stream = FileStreamAndroid::create(fullPath.c_str(), modeStr);
        }

        return stream;
    }
#else
    std::string fullPath;
    getFullPath(path, fullPath);
    UPtr<Stream> stream = FileStream::create(fullPath.c_str(), modeStr).dynamicCastTo<Stream>();
    return stream;
#endif
}

FILE* FileSystem::openFile(const char* filePath, const char* mode)
{
    GP_ASSERT(filePath);
    GP_ASSERT(mode);

    std::string fullPath;
    getFullPath(filePath, fullPath);

    createFileFromAsset(filePath);
    
    FILE* fp = fopen(fullPath.c_str(), mode);
    return fp;
}

char* FileSystem::readAll(const char* filePath, int* fileSize)
{
    GP_ASSERT(filePath);

    // Open file for reading.
    UPtr<Stream> stream(open(filePath));
    if (stream.get() == NULL)
    {
        GP_ERROR("Failed to load file: %s", filePath);
        return NULL;
    }
    size_t size = stream->length();

    // Read entire file contents.
    char* buffer = new char[size + 1];
    size_t read = stream->read(buffer, 1, size);
    if (read != size)
    {
        GP_ERROR("Failed to read complete contents of file '%s' (amount read vs. file size: %u < %u).", filePath, read, size);
        SAFE_DELETE_ARRAY(buffer);
        return NULL;
    }

    // Force the character buffer to be NULL-terminated.
    buffer[size] = '\0';

    if (fileSize)
    {
        *fileSize = (int)size; 
    }
    return buffer;
}

std::string FileSystem::readAllStr(const char* filePath) {
    GP_ASSERT(filePath);
    std::string result;

    // Open file for reading.
    UPtr<Stream> stream(open(filePath));
    if (stream.get() == NULL)
    {
        GP_ERROR("Failed to load file: %s", filePath);
        return result;
    }
    size_t size = stream->length();

    // Read entire file contents.
    result.resize(size);
    char* buffer = (char*)result.data();
    size_t read = stream->read(buffer, 1, size);
    if (read != size)
    {
        GP_ERROR("Failed to read complete contents of file '%s' (amount read vs. file size: %u < %u).", filePath, read, size);
        result.resize(read);
        return result;
    }

    return result;
}

bool FileSystem::isAbsolutePath(const char* filePath)
{
    if (filePath == 0 || filePath[0] == '\0')
        return false;
#ifdef _WIN32
    if (filePath[1] != '\0')
    {
        char first = filePath[0];
        return (filePath[1] == ':' && ((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z')));
    }
    return false;
#else
    return filePath[0] == '/';
#endif
}

void FileSystem::setAssetPath(const char* path)
{
    __assetPath = path;
}

const char* FileSystem::getAssetPath()
{
    return __assetPath.c_str();
}

void FileSystem::createFileFromAsset(const char* path)
{
#ifdef __ANDROID__
    static std::set<std::string> upToDateAssets;

    GP_ASSERT(path);
    std::string fullPath(__resourcePath);
    std::string resolvedPath = FileSystem::resolvePath(path);
    fullPath += resolvedPath;

    std::string directoryPath = fullPath.substr(0, fullPath.rfind('/'));
    struct stat s;
    if (stat(directoryPath.c_str(), &s) != 0)
        mkdirs(directoryPath);

    // To ensure that the files on the file system corresponding to the assets in the APK bundle
    // are always up to date (and in sync), we copy them from the APK to the file system once
    // for each time the process (game) runs.
    if (upToDateAssets.find(fullPath) == upToDateAssets.end())
    {
        AAsset* asset = AAssetManager_open(__assetManager, resolvedPath.c_str(), AASSET_MODE_RANDOM);
        if (asset)
        {
            const void* data = AAsset_getBuffer(asset);
            int length = AAsset_getLength(asset);
            FILE* file = fopen(fullPath.c_str(), "wb");
            if (file != NULL)
            {
                int ret = fwrite(data, sizeof(unsigned char), length, file);
                if (fclose(file) != 0)
                {
                    GP_ERROR("Failed to close file on file system created from APK asset '%s'.", path);
                    return;
                }
                if (ret != length)
                {
                    GP_ERROR("Failed to write all data from APK asset '%s' to file on file system.", path);
                    return;
                }
            }
            else
            {
                GP_ERROR("Failed to create file on file system from APK asset '%s'.", path);
                return;
            }

            upToDateAssets.insert(fullPath);
        }
    }
#endif
}

std::string FileSystem::getDirectoryName(const char* path)
{
    if (path == NULL || strlen(path) == 0)
    {
        return "";
    }
#ifdef _WIN32
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    _splitpath(path, drive, dir, NULL, NULL);
    std::string dirname;
    size_t driveLength = strlen(drive);
    if (driveLength > 0)
    {
        dirname.reserve(driveLength + strlen(dir));
        dirname.append(drive);
        dirname.append(dir);
    }
    else
    {
        dirname.assign(dir);
    }
    std::replace(dirname.begin(), dirname.end(), '\\', '/');
    return dirname;
#else
    // dirname() modifies the input string so create a temp string
    std::string dirname;
    char* tempPath = new char[strlen(path) + 1];
    strcpy(tempPath, path);
    char* dir = ::dirname(tempPath);
    if (dir && strlen(dir) > 0)
    {
        dirname.assign(dir);
        // dirname() strips off the trailing '/' so add it back to be consistent with Windows
        dirname.append("/");
    }
    SAFE_DELETE_ARRAY(tempPath);
    return dirname;
#endif
}

std::string FileSystem::getExtension(const char* path, bool uppper)
{
    const char* str = strrchr(path, '.');
    if (str == NULL)
        return "";

    std::string ext;
    size_t len = strlen(str);
    for (size_t i = 0; i < len; ++i)
        if (uppper) ext += std::toupper(str[i]);
        else ext += str[i];

    return ext;
}

std::string FileSystem::getParentPath(const char* path) {
    std::string spath = path;
#if _WIN32
    std::replace(spath.begin(), spath.end(), '\\', '/');
#endif

    std::string::size_type pos = spath.find_last_of("/");
    if (pos != std::string::npos && pos + 1 < spath.size()) {
        spath = spath.substr(0, pos);
    }
    return spath;
}

std::string FileSystem::getBaseName(const char* path) {
    std::string spath = path;
#if _WIN32
    std::replace(spath.begin(), spath.end(), '\\', '/');
#endif

    std::string::size_type pos = spath.find_last_of("/");
    if (pos != std::string::npos && pos+1 < spath.size()) {
        spath = spath.substr(pos + 1);
    }

    pos = spath.find_last_of(".");
    if (pos != std::string::npos) {
        spath = spath.substr(0, pos);
    }
    return spath;
}

bool FileSystem::remove(const char* path) {
#ifdef _WIN32
    if (DeleteFileA(path)) {
        return true;
    }
#else
    if (::remove(path) == 0) {
        return true;
    }
#endif
    return false;
}

bool FileSystem::copyFile(const char* src, const char* dst) {
    UPtr<Stream> stream = FileSystem::open(src, FileSystem::READ);
    if (stream.isNull()) return false;
    UPtr<Stream> out = FileSystem::open(dst, FileSystem::WRITE);
    
    int size = stream->length();
    char buffer[1024];
    while (size > 0) {
        int n = stream->read(buffer, 1024);
        out->write(buffer, n);
        size -= n;
    }
    stream->close();
    out->close();
    return true;
}

}
