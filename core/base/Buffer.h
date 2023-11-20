/*
 * Copyright (c) 2012-2016, chunquedong
 *
 * This file is part of cppfan project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
 *
 * History:
 *   2012-12-23  Jed Young  Creation
 */
#ifndef FILESTREAM_H_
#define FILESTREAM_H_

#include "Stream.h"

namespace mgp
{

/**
 * ByteArray is memory buffer
 */
class Buffer : public Stream {
private:
  uint8_t* data;
  size_t _pos;
  size_t _size;
  bool owner;
  
public:
    Buffer();
    Buffer(size_t size);
    Buffer(uint8_t* data, size_t size, bool owner);
    ~Buffer();

    //virtual ssize_t write(const char *buf, size_t size) override;
    //virtual ssize_t read(char *buf, size_t size) override;

    void readSlice(Buffer &out, bool copy);

    //read Data no copy
    unsigned char * readDirect(int len);
    unsigned char *getData() { return data; }

    size_t remaining() { return _size - _pos; }

    virtual size_t read(void* ptr, size_t size, size_t count);
    virtual size_t write(const void* ptr, size_t size, size_t count);
    virtual size_t length() { return _size; }
    virtual long int position() { return _pos; }
    virtual bool seek(long int offset, int origin = SEEK_SET);
};


}

#endif