// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_BYTEBUFFER_H
#define HNRT_BYTEBUFFER_H


#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include "Atomic.h"


namespace hnrt
{
    class ByteBuffer
    {
    public:

        inline ByteBuffer(size_t = 0);
        inline ByteBuffer(const void*, size_t);
        inline ByteBuffer(const ByteBuffer&);
        inline ~ByteBuffer();
        inline unsigned char* rPtr() const;
        inline unsigned char* wPtr() const;
        inline int64_t capacity() const;
        inline int64_t wPos() const;
        inline int64_t rPos() const;
        inline int64_t remaining() const;
        inline int64_t space() const;
        inline void capacity(size_t);
        inline void wPos(size_t);
        inline void rPos(size_t);
        inline void clear();
        inline void push();
        inline const unsigned char& operator [](size_t) const;
        inline unsigned char& operator [](size_t);
        inline unsigned char peekU8(size_t) const;
        inline unsigned short peekU16(size_t) const;
        inline unsigned int peekU32(size_t) const;
        inline signed int peekS32(size_t) const;
        inline void peek(void*, size_t) const;
        inline unsigned char getU8();
        inline unsigned short getU16();
        inline unsigned int getU32();
        inline signed int getS32();
        inline void get(void*, size_t);
        inline void put(unsigned char);
        inline void put(unsigned short);
        inline void put(unsigned int);
        inline void put(signed int);
        inline void put(const void*, size_t);

    private:

        unsigned char* _ptr;
        int64_t _capacity;
        volatile int64_t _wPos;
        volatile int64_t _rPos;
    };

    inline ByteBuffer::ByteBuffer(size_t capacity)
        : _ptr(0)
        , _capacity(0)
        , _wPos(0)
        , _rPos(0)
    {
        if (capacity)
        {
            _ptr = (unsigned char*)malloc(capacity);
            if (!_ptr)
            {
                throw std::bad_alloc();
            }
            _capacity = capacity;
        }
    }

    inline ByteBuffer::ByteBuffer(const void* value, size_t length)
        : _ptr(0)
        , _capacity(0)
        , _wPos(0)
        , _rPos(0)
    {
        if (length)
        {
            _ptr = (unsigned char*)malloc(length);
            if (!_ptr)
            {
                throw std::bad_alloc();
            }
            if (value)
            {
                memcpy(_ptr, value, length);
            }
            else
            {
                memset(_ptr, 0, length);
            }
            _wPos = _capacity = length;
        }
    }

    inline ByteBuffer::ByteBuffer(const ByteBuffer& src)
        : _ptr(0)
        , _capacity(0)
        , _wPos(0)
        , _rPos(0)
    {
        if (src._capacity)
        {
            _ptr = (unsigned char*)malloc(src._capacity);
            if (!_ptr)
            {
                throw std::bad_alloc();
            }
            memcpy(_ptr, src._ptr, src._capacity);
            _capacity = src._capacity;
            _wPos = src._wPos;
            _rPos = src._rPos;
        }
    }

    inline ByteBuffer::~ByteBuffer()
    {
        free(_ptr);
    }

    // _wPos must not be changed while in this method.
    inline void ByteBuffer::capacity(size_t requested)
    {
        if (static_cast<size_t>(_capacity) < requested)
        {
            unsigned char* ptr = (unsigned char*)malloc(requested);
            if (!ptr)
            {
                throw std::bad_alloc();
            }
            memcpy(ptr, _ptr, _wPos);
            ptr = InterlockedExchangePointer(&_ptr, ptr);
            free(ptr);
            InterlockedExchange(&_capacity, requested);
        }
    }

    inline void ByteBuffer::wPos(size_t requested)
    {
        if (static_cast<size_t>(_capacity) < requested)
        {
            throw std::invalid_argument("ByteBuffer::wPos: Out of bounds.");
        }
        _wPos = requested;
        if (_rPos > _wPos)
        {
            _rPos = _wPos;
        }
    }

    inline void ByteBuffer::rPos(size_t requested)
    {
        if (static_cast<size_t>(_wPos) < requested)
        {
            throw std::invalid_argument("ByteBuffer::rPos: Out of bounds.");
        }
        _rPos = requested;
    }

    inline unsigned char* ByteBuffer::rPtr() const
    {
        return _ptr + _rPos;
    }

    inline unsigned char* ByteBuffer::wPtr() const
    {
        return _ptr + _wPos;
    }

    inline int64_t ByteBuffer::capacity() const
    {
        return _capacity;
    }

    inline int64_t ByteBuffer::wPos() const
    {
        return _wPos;
    }

    inline int64_t ByteBuffer::rPos() const
    {
        return _rPos;
    }

    inline int64_t ByteBuffer::remaining() const
    {
        return _wPos - _rPos;
    }

    inline int64_t ByteBuffer::space() const
    {
        return _capacity - _wPos;
    }

    inline void ByteBuffer::clear()
    {
        _wPos = 0;
        _rPos = 0;
    }

    inline void ByteBuffer::push()
    {
        if (_rPos)
        {
            ssize_t remaining = _wPos - _rPos;
            if (remaining > 0)
            {
                memmove(_ptr, _ptr + _rPos, remaining);
            }
            else
            {
                remaining = 0;
            }
            _rPos = 0;
            _wPos = remaining;
        }
    }

    inline const unsigned char& ByteBuffer::operator [](size_t index) const
    {
        if (index < static_cast<size_t>(_wPos))
        {
            return _ptr[index];
        }
        else
        {
            throw std::out_of_range("ByteBuffer::operator[]");
        }
    }

    inline unsigned char& ByteBuffer::operator [](size_t index)
    {
        if (index < static_cast<size_t>(_wPos))
        {
            return _ptr[index];
        }
        else
        {
            throw std::out_of_range("ByteBuffer::operator[]");
        }
    }

    inline unsigned char ByteBuffer::peekU8(size_t offset) const
    {
        size_t index = _rPos + offset;
        return _ptr[index];
    }

    inline unsigned short ByteBuffer::peekU16(size_t offset) const
    {
        size_t index = _rPos + offset;
        unsigned short value
            = (static_cast<unsigned short>(_ptr[index + 0]) << (8 * 1))
            | (static_cast<unsigned short>(_ptr[index + 1]) << (8 * 0));
        return value;
    }

    inline unsigned int ByteBuffer::peekU32(size_t offset) const
    {
        size_t index = _rPos + offset;
        unsigned int value
            = (static_cast<unsigned int>(_ptr[index + 0]) << (8 * 3))
            | (static_cast<unsigned int>(_ptr[index + 1]) << (8 * 2))
            | (static_cast<unsigned int>(_ptr[index + 2]) << (8 * 1))
            | (static_cast<unsigned int>(_ptr[index + 3]) << (8 * 0));
        return value;
    }

    inline signed int ByteBuffer::peekS32(size_t offset) const
    {
        size_t index = _rPos + offset;
        signed int value
            = (static_cast<signed int>(_ptr[index + 0]) << (8 * 3))
            | (static_cast<signed int>(_ptr[index + 1]) << (8 * 2))
            | (static_cast<signed int>(_ptr[index + 2]) << (8 * 1))
            | (static_cast<signed int>(_ptr[index + 3]) << (8 * 0));
        return value;
    }

    inline void ByteBuffer::peek(void* value, size_t length) const
    {
        memcpy(value, &_ptr[_rPos], length);
    }

    inline unsigned char ByteBuffer::getU8()
    {
        unsigned char value = _ptr[_rPos];
        _rPos++;
        return value;
    }

    inline unsigned short ByteBuffer::getU16()
    {
        unsigned short value
            = (static_cast<unsigned short>(_ptr[_rPos + 0]) << (8 * 1))
            | (static_cast<unsigned short>(_ptr[_rPos + 1]) << (8 * 0));
        _rPos += 2;
        return value;
    }

    inline unsigned int ByteBuffer::getU32()
    {
        unsigned int value
            = (static_cast<unsigned int>(_ptr[_rPos + 0]) << (8 * 3))
            | (static_cast<unsigned int>(_ptr[_rPos + 1]) << (8 * 2))
            | (static_cast<unsigned int>(_ptr[_rPos + 2]) << (8 * 1))
            | (static_cast<unsigned int>(_ptr[_rPos + 3]) << (8 * 0));
        _rPos += 4;
        return value;
    }

    inline signed int ByteBuffer::getS32()
    {
        signed int value
            = (static_cast<signed int>(_ptr[_rPos + 0]) << (8 * 3))
            | (static_cast<signed int>(_ptr[_rPos + 1]) << (8 * 2))
            | (static_cast<signed int>(_ptr[_rPos + 2]) << (8 * 1))
            | (static_cast<signed int>(_ptr[_rPos + 3]) << (8 * 0));
        _rPos += 4;
        return value;
    }

    inline void ByteBuffer::get(void* value, size_t length)
    {
        memcpy(value, &_ptr[_rPos], length);
        _rPos += static_cast<int>(length);
    }

    inline void ByteBuffer::put(unsigned char value)
    {
        _ptr[_wPos] = value;
        _wPos++;
    }

    inline void ByteBuffer::put(unsigned short value)
    {
        _ptr[_wPos + 0] = (value >> (8 * 1)) & 0xff;
        _ptr[_wPos + 1] = (value >> (8 * 0)) & 0xff;
        _wPos += 2;
    }

    inline void ByteBuffer::put(unsigned int value)
    {
        _ptr[_wPos + 0] = (value >> (8 * 3)) & 0xff;
        _ptr[_wPos + 1] = (value >> (8 * 2)) & 0xff;
        _ptr[_wPos + 2] = (value >> (8 * 1)) & 0xff;
        _ptr[_wPos + 3] = (value >> (8 * 0)) & 0xff;
        _wPos += 4;
    }

    inline void ByteBuffer::put(signed int value)
    {
        _ptr[_wPos + 0] = (value >> (8 * 3)) & 0xff;
        _ptr[_wPos + 1] = (value >> (8 * 2)) & 0xff;
        _ptr[_wPos + 2] = (value >> (8 * 1)) & 0xff;
        _ptr[_wPos + 3] = (value >> (8 * 0)) & 0xff;
        _wPos += 4;
    }

    inline void ByteBuffer::put(const void* value, size_t length)
    {
        memcpy(&_ptr[_wPos], value, length);
        _wPos += static_cast<int64_t>(length);
    }
}


#endif //!HNRT_BYTEBUFFER_H
