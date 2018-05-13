// Copyright (C) 2017-2018 Hideaki Narita


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
    // Integer values are stored in the network order (big endian).
    class ByteBuffer
    {
    public:

        inline ByteBuffer(size_t = 0);
        inline ByteBuffer(const void*, size_t);
        inline ByteBuffer(const ByteBuffer&);
        inline ~ByteBuffer();
        inline int64_t capacity() const;
        inline unsigned char* rPtr() const;
        inline unsigned char* wPtr() const;
        inline int64_t rPos() const;
        inline int64_t wPos() const;
        inline int64_t rLen() const;
        inline int64_t wLen() const;
        inline bool canRead() const;
        inline bool canWrite() const;
        inline void capacity(size_t);
        inline void wPos(size_t);
        inline void rPos(size_t);
        inline void clear();
        inline bool push();
        inline bool tryRewind();
        inline const unsigned char& operator [](size_t) const;
        inline unsigned char& operator [](size_t);
        inline void peek(unsigned char&, size_t) const;
        inline void peek(unsigned short&, size_t) const;
        inline void peek(unsigned int&, size_t) const;
        inline void peek(signed int&, size_t) const;
        inline void peek(void*, size_t) const;
        inline void read(unsigned char&);
        inline void read(unsigned short&);
        inline void read(unsigned int&);
        inline void read(signed int&);
        inline void read(void*, size_t);
        inline unsigned char peekU8(size_t) const;
        inline unsigned short peekU16(size_t) const;
        inline unsigned int peekU32(size_t) const;
        inline signed int peekS32(size_t) const;
        inline unsigned char readU8();
        inline unsigned short readU16();
        inline unsigned int readU32();
        inline signed int readS32();
        inline void write(unsigned char);
        inline void write(unsigned short);
        inline void write(unsigned int);
        inline void write(signed int);
        inline void write(const void*, size_t);

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

    inline int64_t ByteBuffer::capacity() const
    {
        return _capacity;
    }

    inline unsigned char* ByteBuffer::rPtr() const
    {
        return _ptr + _rPos;
    }

    inline unsigned char* ByteBuffer::wPtr() const
    {
        return _ptr + _wPos;
    }

    inline int64_t ByteBuffer::rPos() const
    {
        return _rPos;
    }

    inline int64_t ByteBuffer::wPos() const
    {
        return _wPos;
    }

    inline int64_t ByteBuffer::rLen() const
    {
        return _wPos - _rPos;
    }

    inline int64_t ByteBuffer::wLen() const
    {
        return _capacity - _wPos;
    }

    inline bool ByteBuffer::canRead() const
    {
        return rLen() > 0;
    }

    inline bool ByteBuffer::canWrite() const
    {
        return wLen() > 0;
    }

    // IMPORTANT: _wPos must not be changed while in this method.
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
            free(InterlockedExchangePointer(&_ptr, ptr));
            _capacity = requested;
        }
    }

    inline void ByteBuffer::wPos(size_t requested)
    {
        if (static_cast<size_t>(_capacity) < requested)
        {
            throw std::invalid_argument("ByteBuffer::wPos: Out of bounds.");
        }
        _wPos = requested;
    }

    inline void ByteBuffer::rPos(size_t requested)
    {
        if (static_cast<size_t>(_wPos) < requested)
        {
            throw std::invalid_argument("ByteBuffer::rPos: Out of bounds.");
        }
        _rPos = requested;
    }

    // IMPORTANT: Be sure that the caller has an exclusive access to the buffer.
    inline void ByteBuffer::clear()
    {
        _wPos = 0;
        _rPos = 0;
    }

    // IMPORTANT: Be sure that the caller has an exclusive access to the buffer.
    inline bool ByteBuffer::push()
    {
        if (_rPos)
        {
            int64_t remaining = rLen();
            if (remaining > 0)
            {
                memmove(_ptr, _ptr + _rPos, remaining);
                _rPos = 0;
                _wPos = remaining;
            }
            else
            {
                _rPos = 0;
                _wPos = 0;
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    // IMPORTANT: Be sure that the caller has an exclusive access to the buffer.
    inline bool ByteBuffer::tryRewind()
    {
        if (InterlockedCompareExchange(&_wPos, 0, _rPos) == _rPos)
        {
            _rPos = 0;
            return true;
        }
        else
        {
            return false;
        }
    }

    inline const unsigned char& ByteBuffer::operator [](size_t index) const
    {
        if (index < static_cast<size_t>(_capacity))
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
        if (index < static_cast<size_t>(_capacity))
        {
            return _ptr[index];
        }
        else
        {
            throw std::out_of_range("ByteBuffer::operator[]");
        }
    }

    inline void ByteBuffer::peek(unsigned char& value, size_t offset) const
    {
        size_t index = _rPos + offset;
        value = _ptr[index];
    }

    inline void ByteBuffer::peek(unsigned short& value, size_t offset) const
    {
        size_t index = _rPos + offset;
        value
            = (static_cast<unsigned short>(_ptr[index + 0]) << (8 * 1))
            | (static_cast<unsigned short>(_ptr[index + 1]) << (8 * 0));
    }

    inline void ByteBuffer::peek(unsigned int& value, size_t offset) const
    {
        size_t index = _rPos + offset;
        value
            = (static_cast<unsigned int>(_ptr[index + 0]) << (8 * 3))
            | (static_cast<unsigned int>(_ptr[index + 1]) << (8 * 2))
            | (static_cast<unsigned int>(_ptr[index + 2]) << (8 * 1))
            | (static_cast<unsigned int>(_ptr[index + 3]) << (8 * 0));
    }

    inline void ByteBuffer::peek(signed int& value, size_t offset) const
    {
        size_t index = _rPos + offset;
        value
            = (static_cast<signed int>(_ptr[index + 0]) << (8 * 3))
            | (static_cast<signed int>(_ptr[index + 1]) << (8 * 2))
            | (static_cast<signed int>(_ptr[index + 2]) << (8 * 1))
            | (static_cast<signed int>(_ptr[index + 3]) << (8 * 0));
    }

    inline void ByteBuffer::peek(void* value, size_t length) const
    {
        memcpy(value, &_ptr[_rPos], length);
    }

    inline void ByteBuffer::read(unsigned char& value)
    {
        value = _ptr[_rPos];
        _rPos++;
    }

    inline void ByteBuffer::read(unsigned short& value)
    {
        value
            = (static_cast<unsigned short>(_ptr[_rPos + 0]) << (8 * 1))
            | (static_cast<unsigned short>(_ptr[_rPos + 1]) << (8 * 0));
        _rPos += 2;
    }

    inline void ByteBuffer::read(unsigned int& value)
    {
        value
            = (static_cast<unsigned int>(_ptr[_rPos + 0]) << (8 * 3))
            | (static_cast<unsigned int>(_ptr[_rPos + 1]) << (8 * 2))
            | (static_cast<unsigned int>(_ptr[_rPos + 2]) << (8 * 1))
            | (static_cast<unsigned int>(_ptr[_rPos + 3]) << (8 * 0));
        _rPos += 4;
    }

    inline void ByteBuffer::read(signed int& value)
    {
        value
            = (static_cast<signed int>(_ptr[_rPos + 0]) << (8 * 3))
            | (static_cast<signed int>(_ptr[_rPos + 1]) << (8 * 2))
            | (static_cast<signed int>(_ptr[_rPos + 2]) << (8 * 1))
            | (static_cast<signed int>(_ptr[_rPos + 3]) << (8 * 0));
        _rPos += 4;
    }

    inline void ByteBuffer::read(void* value, size_t length)
    {
        memcpy(value, &_ptr[_rPos], length);
        _rPos += static_cast<int64_t>(length);
    }

    // peekXX and readXX methods are provided for assigning a value to a packed field.

    inline unsigned char ByteBuffer::peekU8(size_t offset) const
    {
        size_t index = _rPos + offset;
        unsigned char value = _ptr[index];
        return value;
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

    inline unsigned char ByteBuffer::readU8()
    {
        unsigned char value = _ptr[_rPos];
        _rPos++;
        return value;
    }

    inline unsigned short ByteBuffer::readU16()
    {
        unsigned short value
            = (static_cast<unsigned short>(_ptr[_rPos + 0]) << (8 * 1))
            | (static_cast<unsigned short>(_ptr[_rPos + 1]) << (8 * 0));
        _rPos += 2;
        return value;
    }

    inline unsigned int ByteBuffer::readU32()
    {
        unsigned int value
            = (static_cast<unsigned int>(_ptr[_rPos + 0]) << (8 * 3))
            | (static_cast<unsigned int>(_ptr[_rPos + 1]) << (8 * 2))
            | (static_cast<unsigned int>(_ptr[_rPos + 2]) << (8 * 1))
            | (static_cast<unsigned int>(_ptr[_rPos + 3]) << (8 * 0));
        _rPos += 4;
        return value;
    }

    inline signed int ByteBuffer::readS32()
    {
        signed int value
            = (static_cast<signed int>(_ptr[_rPos + 0]) << (8 * 3))
            | (static_cast<signed int>(_ptr[_rPos + 1]) << (8 * 2))
            | (static_cast<signed int>(_ptr[_rPos + 2]) << (8 * 1))
            | (static_cast<signed int>(_ptr[_rPos + 3]) << (8 * 0));
        _rPos += 4;
        return value;
    }

    inline void ByteBuffer::write(unsigned char value)
    {
        _ptr[_wPos] = value;
        _wPos++;
    }

    inline void ByteBuffer::write(unsigned short value)
    {
        _ptr[_wPos + 0] = (value >> (8 * 1)) & 0xff;
        _ptr[_wPos + 1] = (value >> (8 * 0)) & 0xff;
        _wPos += 2;
    }

    inline void ByteBuffer::write(unsigned int value)
    {
        _ptr[_wPos + 0] = (value >> (8 * 3)) & 0xff;
        _ptr[_wPos + 1] = (value >> (8 * 2)) & 0xff;
        _ptr[_wPos + 2] = (value >> (8 * 1)) & 0xff;
        _ptr[_wPos + 3] = (value >> (8 * 0)) & 0xff;
        _wPos += 4;
    }

    inline void ByteBuffer::write(signed int value)
    {
        _ptr[_wPos + 0] = (value >> (8 * 3)) & 0xff;
        _ptr[_wPos + 1] = (value >> (8 * 2)) & 0xff;
        _ptr[_wPos + 2] = (value >> (8 * 1)) & 0xff;
        _ptr[_wPos + 3] = (value >> (8 * 0)) & 0xff;
        _wPos += 4;
    }

    inline void ByteBuffer::write(const void* value, size_t length)
    {
        memcpy(&_ptr[_wPos], value, length);
        _wPos += static_cast<int64_t>(length);
    }
}


#endif //!HNRT_BYTEBUFFER_H
