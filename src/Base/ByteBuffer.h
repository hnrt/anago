// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_BYTEBUFFER_H
#define HNRT_BYTEBUFFER_H


#include <stddef.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class ByteBuffer
        : public RefObj
    {
    public:

        static RefPtr<ByteBuffer> create(size_t = 0);
        static RefPtr<ByteBuffer> create(const void*, size_t);

        ByteBuffer(size_t = 0);
        ByteBuffer(const void*, size_t);
        ByteBuffer(const ByteBuffer&);
        ~ByteBuffer();
        unsigned char* ptr() const { return _ptr; }
        unsigned char* cur() const { return _ptr + _position; }
        int capacity() const { return static_cast<int>(_capacity); }
        int limit() const { return static_cast<int>(_limit); }
        int position() const { return static_cast<int>(_position); }
        bool hasRemaining() const { return _position < _limit; }
        int remaining() const { return static_cast<int>(_limit - _position); }
        ByteBuffer& capacity(size_t);
        ByteBuffer& limit(size_t);
        ByteBuffer& position(size_t);
        ByteBuffer& clear();
        ByteBuffer& flip();
        ByteBuffer& rewind();
        ByteBuffer& prepareForRead();
        ByteBuffer& set(const void*, size_t);
        const unsigned char& operator [](size_t) const;
        unsigned char& operator [](size_t);
        unsigned char get();
        ByteBuffer& put(unsigned char);
        ByteBuffer& put(unsigned short);
        ByteBuffer& put(unsigned int);
        ByteBuffer& put(signed int);
        ByteBuffer& put(const void*, size_t);

    private:

        unsigned char* _ptr;
        size_t _capacity;
        size_t _limit;
        size_t _position;
    };
}


#endif //!HNRT_BYTEBUFFER_H
