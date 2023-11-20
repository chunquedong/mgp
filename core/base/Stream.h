#ifndef Stream_H_
#define Stream_H_

#include <stdint.h>
#include "Base.h"

namespace mgp
{
    /**
     * ByteOrder
     */
    class Endian {
    public:
        enum Type {
            Big,
            Little
        };

        static inline Type hostEndian() {
            static const int __one = 1;
            return (*(char*)(&__one) == 0) ? Endian::Big : Endian::Little;
        }

        static inline void swap16p(void* lpMem) {
            char* p = (char*)lpMem;
            p[0] = p[0] ^ p[1];
            p[1] = p[0] ^ p[1];
            p[0] = p[0] ^ p[1];
        }

        static inline void swap32p(void* lpMem) {
            char* p = (char*)lpMem;
            p[0] = p[0] ^ p[3];
            p[3] = p[0] ^ p[3];
            p[0] = p[0] ^ p[3];
            p[1] = p[1] ^ p[2];
            p[2] = p[1] ^ p[2];
            p[1] = p[1] ^ p[2];
        }

        static inline void swap64p(void* lpMem) {
            char* p = (char*)lpMem;
            p[0] = p[0] ^ p[7];
            p[7] = p[0] ^ p[7];
            p[0] = p[0] ^ p[7];
            p[1] = p[1] ^ p[6];
            p[6] = p[1] ^ p[6];
            p[1] = p[1] ^ p[6];
            p[2] = p[2] ^ p[5];
            p[5] = p[2] ^ p[5];
            p[2] = p[2] ^ p[5];
            p[3] = p[3] ^ p[4];
            p[4] = p[3] ^ p[4];
            p[3] = p[3] ^ p[4];
        }

    };
/**
 * Defines a stream for reading and writing a sequence of bytes.
 * 
 * Use FileSystem::open() to create a stream.
 * 
 * @script{ignore}
 */
class Stream
{
public:

    /**
     * Destructor. The stream should be closed when it is destroyed.
     */
    virtual ~Stream() {};

    /**
     * Returns true if this stream can perform read operations.
     * 
     * @return True if the stream can read, false otherwise.
     */
    virtual bool canRead() { return true; }

    /**
     * Returns true if this stream can perform write operations.
     * 
     * @return True if the stream can write, false otherwise.
     */
    virtual bool canWrite() { return true; }

    /**
     * Returns true if this stream can seek.
     * 
     * @return True if the stream can seek, false otherwise.
     */
    virtual bool canSeek() { return true; }
    
    /**
     * Reads an array of <code>count</code> elements, each of size <code>size</code>.
     * 
     * \code
     * int numbers[3];
     * if (stream->read(numbers, sizeof(int), 3) != 3)
     *     print("Error reading from file");
     * \endcode
     * 
     * @param ptr   The pointer to the memory to copy into.
     *              The available size should be at least (<code>size * count</code>) bytes.
     * @param size  The size of each element to be read, in bytes.
     * @param count The number of elements to read.
     * 
     * @return The number of elements read.
     * 
     * @see canRead()
     */
    virtual size_t read(void* ptr, size_t size, size_t count) = 0;

    /**
     * Reads a line from the stream.
     * 
     * A new line is denoted by by either "\n", "\r" or "\r\n".
     * The line break character is included in the string.
     * The terminating null character is added to the end of the string.
     * 
     * @param str The array of chars to copy the string to.
     * @param num The maximum number of characters to be copied.
     * 
     * @return On success, str is returned. On error, NULL is returned.
     * 
     * @see canRead()
     */
    virtual char* readLine(char* str, int num);
    
    /**
     * Writes an array of <code>count</code> elements, each of size <code>size</code>.
     * 
     * \code
     * int numbers[] = {1, 2, 3};
     * if (stream->write(numbers, sizeof(int), 3) != 3)
     *     print("Error writing to file");
     * \endcode
     * 
     * @param ptr   The pointer to the array of elements to be written.
     * @param size  The size of each element to be written, in bytes.
     * @param count The number of elements to write.
     * 
     * @return The number of elements written.
     * 
     * @see canWrite()
     */
    virtual size_t write(const void* ptr, size_t size, size_t count) = 0;

    /**
     * Returns true if the end of the stream has been reached.
     * 
     * @return True if end of stream reached, false otherwise.
     */
    virtual bool eof();

    /**
     * Returns the length of the stream in bytes.
     * 
     * Zero is returned if the length of the stream is unknown and/or it cannot be seeked. 
     * 
     * Example: The length of a network stream is unknown and cannot be seeked.
     * 
     * @return The length of the stream in bytes.
     */
    virtual size_t length() = 0;

    /**
     * Returns the position of the file pointer. Zero is the start of the stream.
     * 
     * @return The file indicator offset in bytes. 
     */
    virtual long int position() = 0;

    /**
     * Sets the position of the file pointer.
     * 
     * Use canSeek() to determine if this method is supported.
     * 
     * @param offset The number of bytes to offset from origin.
     * @param origin The position used as a reference for offset.
     *               The supported values are the same as fseek().
     *                - <code>SEEK_SET</code> relative to the beginning of the file.
     *                - <code>SEEK_CUR</code> relative to the current position of the file pointer.
     *                - <code>SEEK_END</code> relative to the end of file.
     * 
     * @return True if successful, false otherwise.
     * 
     * @see canSeek()
     */
    virtual bool seek(long int offset, int origin = SEEK_SET) = 0;

    /**
     * Moves the file pointer to the start of the file.
     * 
     * Use canSeek() to determine if this method is supported.
     * 
     * @return True if successful, false otherwise.
     * 
     * @see canSeek()
     */
    virtual bool rewind();

protected:
    Stream();
private:
    Stream(const Stream&);            // Hidden copy constructor.
    Stream& operator=(const Stream&); // Hidden copy assignment operator.
public:
    Endian::Type endian;

#ifndef ssize_t
    #define ssize_t int
#endif
    virtual ssize_t write(const char* buf, size_t size);

    /**
     * @return the total number of bytes read into the buffer,
     * or -1 if there is no more data because the end of the stream has been reached.
     */
    virtual ssize_t read(char* buf, size_t size);

    virtual void flush() {}

    virtual void close() {}

    virtual ssize_t pipeTo(Stream* out);

    ssize_t writeInt8(int8_t out);
    ssize_t writeInt16(int16_t out);
    ssize_t writeInt32(int32_t out);
    ssize_t writeInt64(int64_t out);
    ssize_t writeUInt8(uint8_t out);
    ssize_t writeUInt16(uint16_t out);
    ssize_t writeUInt32(uint32_t out);
    ssize_t writeUInt64(uint64_t out);
    ssize_t writeFloat(float out);
    ssize_t writeDouble(double out);

    uint8_t readUInt8();
    uint16_t readUInt16();
    uint32_t readUInt32();
    uint64_t readUInt64();
    int8_t readInt8();
    int16_t readInt16();
    int32_t readInt32();
    int64_t readInt64();
    float readFloat();
    double readDouble();

    void writeStr(const std::string& buf);
    std::string readStr();
};


}

#endif
