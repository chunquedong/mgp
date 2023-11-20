#include "Stream.h"

using namespace mgp;

Stream::Stream(): endian(Endian::Little) {

}

ssize_t Stream::write(const char* buf, size_t size) {
    return this->write(buf, 1, size);
}

/**
 * @return the total number of bytes read into the buffer,
 * or -1 if there is no more data because the end of the stream has been reached.
 */
ssize_t Stream::read(char* buf, size_t size) {
    return this->read(buf, 1, size);
}

ssize_t Stream::pipeTo(Stream* out) {
    ssize_t size = 0;
    ssize_t succSize = 0;
    char buffer[1024];
    ssize_t err;

    while (true) {
        size = this->read(buffer, 1024);
        if (size <= 0) {
            break;
        }
        err = out->write(buffer, size);
        if (err != -1) {
            return succSize;
        }
        succSize += size;
    }

    return succSize;
}

ssize_t Stream::writeUInt8(uint8_t out) {
    static const int size = 1;
    uint8_t data[size];
    data[0] = out;
    ssize_t s = this->write((char*)data, size);
    return s;
}

ssize_t Stream::writeUInt16(uint16_t out) {
    static const int size = 2;
    unsigned char data[size];

    if (endian == Endian::Big) {
        data[0] = (out >> 8) & 0xff;
        data[1] = (out) & 0xff;
    }
    else {
        data[1] = (out >> 8) & 0xff;
        data[0] = (out) & 0xff;
    }

    ssize_t s = this->write((char*)data, size);
    return s;
}

ssize_t Stream::writeUInt32(uint32_t out) {
    static const int size = 4;
    unsigned char data[size];
    if (endian == Endian::Big) {
        data[0] = (out >> 24) & 0xff;
        data[1] = (out >> 16) & 0xff;
        data[2] = (out >> 8) & 0xff;
        data[3] = (out) & 0xff;
    }
    else {
        data[3] = (out >> 24) & 0xff;
        data[2] = (out >> 16) & 0xff;
        data[1] = (out >> 8) & 0xff;
        data[0] = (out) & 0xff;
    }
    ssize_t s = this->write((char*)data, size);
    return s;
}

ssize_t Stream::writeUInt64(uint64_t out) {
    static const int size = 8;
    unsigned char data[size];
    if (endian == Endian::Big) {
        data[0] = (out >> 56) & 0xff;
        data[1] = (out >> 48) & 0xff;
        data[2] = (out >> 40) & 0xff;
        data[3] = (out >> 32) & 0xff;
        data[4] = (out >> 24) & 0xff;
        data[5] = (out >> 16) & 0xff;
        data[6] = (out >> 8) & 0xff;
        data[7] = (out) & 0xff;
    }
    else {
        data[7] = (out >> 56) & 0xff;
        data[6] = (out >> 48) & 0xff;
        data[5] = (out >> 40) & 0xff;
        data[4] = (out >> 32) & 0xff;
        data[3] = (out >> 24) & 0xff;
        data[2] = (out >> 16) & 0xff;
        data[1] = (out >> 8) & 0xff;
        data[0] = (out) & 0xff;
    }
    ssize_t s = this->write((char*)data, size);
    return s;
}

uint8_t Stream::readUInt8() {
    static const int size = 1;
    unsigned char data[size];
    ssize_t s = this->read((char*)data, size);
    if (s < size) return -1;
    return data[0];
}

uint16_t Stream::readUInt16() {
    static const int size = 2;
    unsigned char data[size];
    ssize_t s = this->read((char*)data, size);
    if (s < size) return -1;
    uint16_t byte1 = data[0];
    uint16_t byte2 = data[1];

    if (endian == Endian::Big) {
        return ((byte1 << 8) | byte2);
    }
    else {
        return ((byte2 << 8) | byte1);
    }
}

uint32_t Stream::readUInt32() {
    static const int size = 4;
    unsigned char data[size];
    ssize_t s = this->read((char*)data, size);
    if (s < size) return -1;

    uint32_t byte1 = data[0];
    uint32_t byte2 = data[1];
    uint32_t byte3 = data[2];
    uint32_t byte4 = data[3];

    if (endian == Endian::Big) {
        return ((byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4);
    }
    else {
        return ((byte4 << 24) | (byte3 << 16) | (byte2 << 8) | byte1);
    }
}

uint64_t Stream::readUInt64() {
    static const int size = 8;
    unsigned char data[size];
    ssize_t s = this->read((char*)data, size);
    if (s < size) return -1;

    uint64_t byte1 = data[0];
    uint64_t byte2 = data[1];
    uint64_t byte3 = data[2];
    uint64_t byte4 = data[3];
    uint64_t byte5 = data[4];
    uint64_t byte6 = data[5];
    uint64_t byte7 = data[6];
    uint64_t byte8 = data[7];

    if (endian == Endian::Big) {
        return ((byte1 << 56) | (byte2 << 48) | (byte3 << 40) | (byte4 << 32)
            | (byte5 << 24) | (byte6 << 16) | (byte7 << 8) | byte8);
    }
    else {
        return ((byte8 << 56) | (byte7 << 48) | (byte6 << 40) | (byte5 << 32)
            | (byte4 << 24) | (byte3 << 16) | (byte2 << 8) | byte1);
    }
}

ssize_t Stream::writeInt8(int8_t out) {
    uint8_t intVal = *((uint8_t*)&out);
    return writeUInt8(intVal);
}

ssize_t Stream::writeInt16(int16_t out) {
    uint16_t intVal = *((uint16_t*)&out);
    return writeUInt16(intVal);
}
ssize_t Stream::writeInt32(int32_t out) {
    uint32_t intVal = *((uint32_t*)&out);
    return writeUInt32(intVal);
}
ssize_t Stream::writeInt64(int64_t out) {
    uint64_t intVal = *((uint64_t*)&out);
    return writeUInt64(intVal);
}
ssize_t Stream::writeFloat(float out) {
    uint32_t intVal = *((uint32_t*)&out);
    return writeUInt32(intVal);
}
ssize_t Stream::writeDouble(double out) {
    uint64_t intVal = *((uint64_t*)&out);
    return writeUInt64(intVal);
}

int8_t Stream::readInt8() {
    uint8_t intVal = this->readUInt8();
    return *((int8_t*)&intVal);
}

int16_t Stream::readInt16() {
    uint16_t intVal = this->readUInt16();
    return *((int16_t*)&intVal);
}

int32_t Stream::readInt32() {
    uint32_t intVal = this->readUInt32();
    return *((int32_t*)&intVal);
}

int64_t Stream::readInt64() {
    uint64_t intVal = this->readUInt64();
    return *((int64_t*)&intVal);
}

float Stream::readFloat() {
    uint32_t intVal = this->readUInt32();
    return *((float*)&intVal);
}

double Stream::readDouble() {
    uint64_t intVal = this->readUInt64();
    return *((double*)&intVal);
}

///////////////////////////////////////////////////////////


void Stream::writeStr(const std::string& buf) {
    size_t size = buf.size();
    writeUInt32((uint32_t)size);
    write(buf.c_str(), size);
}

std::string Stream::readStr() {
    size_t size = readUInt32();
    std::string s;
    s.resize(size);
    read((char*)s.data(), size);
    return (s);
}


char* Stream::readLine(char* str, int num)
{
    if (num <= 0)
        return NULL;
    char c = 0;
    size_t maxCharsToRead = num - 1;
    for (size_t i = 0; i < maxCharsToRead; ++i)
    {
        size_t result = read(&c, 1, 1);
        if (result != 1)
        {
            str[i] = '\0';
            break;
        }
        if (c == '\n')
        {
            str[i] = c;
            str[i + 1] = '\0';
            break;
        }
        else if(c == '\r')
        {
            str[i] = c;
            // next may be '\n'
            size_t pos = position();

            char nextChar = 0;
            if (read(&nextChar, 1, 1) != 1)
            {
                // no more characters
                str[i + 1] = '\0';
                break;
            }
            if (nextChar == '\n')
            {
                if (i == maxCharsToRead - 1)
                {
                    str[i + 1] = '\0';
                    break;
                }
                else
                {
                    str[i + 1] = nextChar;
                    str[i + 2] = '\0';
                    break;
                }
            }
            else
            {
                seek(pos, SEEK_SET);
                str[i + 1] = '\0';
                break;
            }
        }
        str[i] = c;
    }
    return str; // what if first read failed?
}

bool Stream::rewind() {
    if (canSeek()) {
        return seek(0);
    }
    return false;
}

bool Stream::eof() {
    return position() >= length();
}