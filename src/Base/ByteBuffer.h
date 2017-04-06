// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_BYTEBUFFER_H
#define HNRT_BYTEBUFFER_H


#include <stddef.h>


namespace hnrt
{
    class ByteBuffer
    {
    public:

        ByteBuffer(size_t = 0);
        ByteBuffer(const void*, size_t);
        ByteBuffer(const ByteBuffer&);
        ~ByteBuffer();
        unsigned char* ptr() const { return _ptr; }
        unsigned char* cur() const { return _ptr + _position; }
        unsigned char* end() const { return _ptr + _limit; }
        int capacity() const { return _capacity; }
        int limit() const { return _limit; }
        int position() const { return _position; }
        int remaining() const { return _limit - _position; }
        int space() const { return _capacity - _limit; }
        void fixLimit() { _limitFixed = true; }
        ByteBuffer& capacity(size_t);
        ByteBuffer& limit(size_t);
        ByteBuffer& position(size_t);
        ByteBuffer& clear();
        ByteBuffer& move();
        const unsigned char& operator [](size_t) const;
        unsigned char& operator [](size_t);
        unsigned char getU8();
        unsigned short getU16();
        unsigned int getU32();
        signed int getS32();
        ByteBuffer& get(void*, size_t);
        ByteBuffer& put(unsigned char);
        ByteBuffer& put(unsigned short);
        ByteBuffer& put(unsigned int);
        ByteBuffer& put(signed int);
        ByteBuffer& put(const void*, size_t);
        unsigned char peekU8(size_t) const;
        unsigned short peekU16(size_t) const;
        unsigned int peekU32(size_t) const;
        signed int peekS32(size_t) const;

    private:

        unsigned char* _ptr;
        int _capacity;
        volatile int _limit;
        volatile int _position;
        volatile bool _limitFixed;
    };
}


#endif //!HNRT_BYTEBUFFER_H
