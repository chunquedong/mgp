#ifndef FILESTREAM_H_
#define FILESTREAM_H_

#include "Stream.h"
#include <string>
#include "Ptr.h"

namespace mgp
{

/**
 * 
 * @script{ignore}
 */
class FileStream : public Stream
{
public:
    friend class FileSystem;
    
    ~FileStream();
    virtual bool canRead();
    virtual bool canWrite();
    virtual bool canSeek();
    virtual void close();
    virtual size_t read(void* ptr, size_t size, size_t count);
    virtual char* readLine(char* str, int num);
    virtual size_t write(const void* ptr, size_t size, size_t count);
    virtual bool eof();
    virtual size_t length();
    virtual long int position();
    virtual bool seek(long int offset, int origin = SEEK_SET);
    virtual bool rewind();

    static UPtr<FileStream> create(const char* filePath, const char* mode);

private:
    FileStream(FILE* file);

private:
    FILE* _file;
    bool _canRead;
    bool _canWrite;
};

#ifdef __ANDROID__

class AAsset;
/**
 * 
 * @script{ignore}
 */
class FileStreamAndroid : public Stream
{
public:
    friend class FileSystem;
    
    ~FileStreamAndroid();
    virtual bool canRead();
    virtual bool canWrite();
    virtual bool canSeek();
    virtual void close();
    virtual size_t read(void* ptr, size_t size, size_t count);
    virtual char* readLine(char* str, int num);
    virtual size_t write(const void* ptr, size_t size, size_t count);
    virtual bool eof();
    virtual size_t length();
    virtual long int position();
    virtual bool seek(long int offset, int origin);
    virtual bool rewind();

    static FileStreamAndroid* create(const char* filePath, const char* mode);

private:
    FileStreamAndroid(AAsset* asset);

private:
    AAsset* _asset;
};

#endif

}

#endif