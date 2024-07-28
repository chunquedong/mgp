#include "FileStream.h"

using namespace mgp;

FileStream::FileStream(FILE* file)
    : _file(file), _canRead(false), _canWrite(false)
{
    
}

FileStream::~FileStream()
{
    if (_file)
    {
        close();
    }
}

#ifdef  _WIN32
#include <Windows.h>
std::string Utf8ToGbk(const char* src_str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
    wchar_t* wszGBK = new wchar_t[len + 1];
    memset(wszGBK, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
    char* szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
    std::string strTemp(szGBK);
    if (wszGBK) delete[] wszGBK;
    if (szGBK) delete[] szGBK;
    return strTemp;
}
#endif //  _WIN32


OwnPtr<FileStream, true> FileStream::create(const char* filePath, const char* mode)
{
#ifdef  _WIN32
    FILE* file = fopen(Utf8ToGbk(filePath).c_str(), mode);
#else
    FILE* file = fopen(filePath, mode);
#endif
    if (file)
    {
        FileStream* stream = new FileStream(file);
        const char* s = mode;
        while (s != NULL && *s != '\0')
        {
            if (*s == 'r')
                stream->_canRead = true;
            else if (*s == 'w')
                stream->_canWrite = true;
            ++s;
        }

        return UPtr<FileStream>(stream);
    }
    else {
        GP_DEBUG("open file fail:%s", filePath);
    }
    return UPtr<FileStream>(NULL);
}

bool FileStream::canRead()
{
    return _file && _canRead;
}

bool FileStream::canWrite()
{
    return _file && _canWrite;
}

bool FileStream::canSeek()
{
    return _file != NULL;
}

void FileStream::close()
{
    if (_file)
        fclose(_file);
    _file = NULL;
}

size_t FileStream::read(void* ptr, size_t size, size_t count)
{
    if (!_file)
        return 0;
    return fread(ptr, size, count, _file);
}

char* FileStream::readLine(char* str, int num)
{
    if (!_file)
        return 0;
    return fgets(str, num, _file);
}

size_t FileStream::write(const void* ptr, size_t size, size_t count)
{
    if (!_file)
        return 0;
    return fwrite(ptr, size, count, _file);
}

bool FileStream::eof()
{
    if (!_file || feof(_file))
        return true;
    return ((size_t)position()) >= length();
}

size_t FileStream::length()
{
    size_t len = 0;
    if (canSeek())
    {
        long int pos = position();
        if (seek(0, SEEK_END))
        {
            len = position();
        }
        seek(pos, SEEK_SET);
    }
    return len;
}

long int FileStream::position()
{
    if (!_file)
        return -1;
    return ftell(_file);
}

bool FileStream::seek(long int offset, int origin)
{
    if (!_file)
        return false;
    return fseek(_file, offset, origin) == 0;
}

bool FileStream::rewind()
{
    if (canSeek())
    {
        ::rewind(_file);
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////
// Android FileStream
////////////////////////////////////////////////////////////////////////////////////

#ifdef __ANDROID__

#include <android/asset_manager.h>
extern AAssetManager* __assetManager;


FileStreamAndroid::FileStreamAndroid(AAsset* asset)
    : _asset(asset)
{
}

FileStreamAndroid::~FileStreamAndroid()
{
    if (_asset)
        close();
}

FileStreamAndroid* FileStreamAndroid::create(const char* filePath, const char* mode)
{
    AAsset* asset = AAssetManager_open(__assetManager, filePath, AASSET_MODE_RANDOM);
    if (asset)
    {
        FileStreamAndroid* stream = new FileStreamAndroid(asset);
        return stream;
    }
    return NULL;
}

bool FileStreamAndroid::canRead()
{
    return true;
}

bool FileStreamAndroid::canWrite()
{
    return false;
}

bool FileStreamAndroid::canSeek()
{
    return true;
}

void FileStreamAndroid::close()
{
    if (_asset)
        AAsset_close(_asset);
    _asset = NULL;
}

size_t FileStreamAndroid::read(void* ptr, size_t size, size_t count)
{
    int result = AAsset_read(_asset, ptr, size * count);
    return result > 0 ? ((size_t)result) / size : 0;
}

size_t FileStreamAndroid::write(const void* ptr, size_t size, size_t count)
{
    return 0;
}

bool FileStreamAndroid::eof()
{
    return position() >= length();
}

size_t FileStreamAndroid::length()
{
    return (size_t)AAsset_getLength(_asset);
}

long int FileStreamAndroid::position()
{
    return AAsset_getLength(_asset) - AAsset_getRemainingLength(_asset);
}

bool FileStreamAndroid::seek(long int offset, int origin)
{
    return AAsset_seek(_asset, offset, origin) != -1;
}

bool FileStreamAndroid::rewind()
{
    if (canSeek())
    {
        return AAsset_seek(_asset, 0, SEEK_SET) != -1;
    }
    return false;
}

#endif