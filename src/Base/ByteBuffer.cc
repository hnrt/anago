// Copyright (C) 2017 Hideaki Narita


#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include "Atomic.h"
#include "ByteBuffer.h"


using namespace hnrt;


ByteBuffer::ByteBuffer(size_t capacity)
    : _ptr(NULL)
    , _capacity(0)
    , _limit(0)
    , _position(0)
    , _limitFixed(false)
{
    if (capacity)
    {
        _ptr = (unsigned char*)malloc(capacity);
        if (!_ptr)
        {
            throw std::bad_alloc();
        }
        _limit = _capacity = capacity;
    }
}


ByteBuffer::ByteBuffer(const void* value, size_t length)
    : _ptr(NULL)
    , _capacity(0)
    , _limit(0)
    , _position(0)
    , _limitFixed(false)
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
        _limit = _capacity = length;
    }
}


ByteBuffer::ByteBuffer(const ByteBuffer& src)
    : _ptr(NULL)
    , _capacity(0)
    , _limit(0)
    , _position(0)
    , _limitFixed(false)
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
        _limit = src._limit;
        _position = src._position;
        _limitFixed = src._limitFixed;
    }
}


ByteBuffer::~ByteBuffer()
{
    free(_ptr);
}


// _limit must not be changed while in this method.
ByteBuffer& ByteBuffer::capacity(size_t requested)
{
    if (static_cast<int>(requested) > _capacity)
    {
        unsigned char* ptr = (unsigned char*)malloc(requested);
        if (!ptr)
        {
            throw std::bad_alloc();
        }
        memcpy(ptr, _ptr, _limit);
        ptr = InterlockedExchangePointer(&_ptr, ptr);
        free(ptr);
        InterlockedExchange(&_capacity, requested);
    }
    return *this;
}


ByteBuffer& ByteBuffer::limit(size_t requested)
{
    if (static_cast<int>(requested) > _capacity)
    {
        throw std::invalid_argument("ByteBuffer::limit: Out of bounds.");
    }
    _limit = requested;
    if (_position > _limit)
    {
        _position = _limit;
    }
    _limitFixed = false;
    return *this;
}


ByteBuffer& ByteBuffer::position(size_t requested)
{
    if (static_cast<int>(requested) > _limit)
    {
        throw std::invalid_argument("ByteBuffer::position: Out of bounds.");
    }
    _position = requested;
    return *this;
}


ByteBuffer& ByteBuffer::clear()
{
    _limit = 0;
    _position = 0;
    return *this;
}


ByteBuffer& ByteBuffer::move()
{
    if (_position)
    {
        size_t remaining = _limit - _position;
        memmove(_ptr, _ptr + _position, remaining);
        _position = 0;
        _limit = remaining;
    }
    return *this;
}


const unsigned char& ByteBuffer::operator [](size_t index) const
{
    if (static_cast<int>(index) < _limit)
    {
        return _ptr[index];
    }
    else
    {
        throw std::out_of_range("ByteBuffer::operator[]");
    }
}


unsigned char& ByteBuffer::operator [](size_t index)
{
    if (static_cast<int>(index) < _limit)
    {
        return _ptr[index];
    }
    else
    {
        throw std::out_of_range("ByteBuffer::operator[]");
    }
}


unsigned char ByteBuffer::getU8()
{
    if (_position + 1 <= _limit)
    {
        unsigned char value = _ptr[_position++];
        if (!_limitFixed)
        {
            if (InterlockedCompareExchange(&_limit, 0, _position) == _position)
            {
                _position = 0;
            }
        }
        return value;
    }
    else
    {
        throw std::out_of_range("ByteBuffer::getU8");
    }
}


unsigned short ByteBuffer::getU16()
{
    if (_position + 2 <= _limit)
    {
        unsigned short value
            = ((unsigned short)_ptr[_position + 0]) << (8 * 1)
            | ((unsigned short)_ptr[_position + 1]) << (8 * 0);
        _position += 2;
        if (!_limitFixed)
        {
            if (InterlockedCompareExchange(&_limit, 0, _position) == _position)
            {
                _position = 0;
            }
        }
        return value;
    }
    else
    {
        throw std::out_of_range("ByteBuffer::getU16");
    }
}


unsigned int ByteBuffer::getU32()
{
    if (_position + 4 <= _limit)
    {
        unsigned int value
            = ((unsigned int)_ptr[_position + 0]) << (8 * 3)
            | ((unsigned int)_ptr[_position + 1]) << (8 * 2)
            | ((unsigned int)_ptr[_position + 2]) << (8 * 1)
            | ((unsigned int)_ptr[_position + 3]) << (8 * 0);
        _position += 4;
        if (!_limitFixed)
        {
            if (InterlockedCompareExchange(&_limit, 0, _position) == _position)
            {
                _position = 0;
            }
        }
        return value;
    }
    else
    {
        throw std::out_of_range("ByteBuffer::getU32");
    }
}


signed int ByteBuffer::getS32()
{
    if (_position + 4 <= _limit)
    {
        signed int value
            = ((signed int)_ptr[_position + 0]) << (8 * 3)
            | ((signed int)_ptr[_position + 1]) << (8 * 2)
            | ((signed int)_ptr[_position + 2]) << (8 * 1)
            | ((signed int)_ptr[_position + 3]) << (8 * 0);
        _position += 4;
        if (!_limitFixed)
        {
            if (InterlockedCompareExchange(&_limit, 0, _position) == _position)
            {
                _position = 0;
            }
        }
        return value;
    }
    else
    {
        throw std::out_of_range("ByteBuffer::getS32");
    }
}


ByteBuffer& ByteBuffer::get(void* value, size_t length)
{
    if (_position + static_cast<int>(length) <= _limit)
    {
        if (value)
        {
            memcpy(value, &_ptr[_position], length);
        }
        _position += static_cast<int>(length);
        if (InterlockedCompareExchange(&_limit, 0, _position) == _position)
        {
            _position = 0;
        }
        return *this;
    }
    else
    {
        throw std::out_of_range("ByteBuffer::get");
    }
}


ByteBuffer& ByteBuffer::put(unsigned char value)
{
    for (;;)
    {
        int limit = _limit;
        if (limit + 1 <= _capacity)
        {
            _ptr[limit] = value;
            if (InterlockedCompareExchange(&_limit, limit + 1, limit) == limit)
            {
                return *this;
            }
        }
        else
        {
            throw std::out_of_range("ByteBuffer::put");
        }
    }
}


ByteBuffer& ByteBuffer::put(unsigned short value)
{
    for (;;)
    {
        int limit = _limit;
        if (limit + 2 <= _capacity)
        {
            _ptr[limit + 0] = (value >> (8 * 1)) & 0xff;
            _ptr[limit + 1] = (value >> (8 * 0)) & 0xff;
            if (InterlockedCompareExchange(&_limit, limit + 2, limit) == limit)
            {
                return *this;
            }
        }
        else
        {
            throw std::out_of_range("ByteBuffer::put");
        }
    }
}


ByteBuffer& ByteBuffer::put(unsigned int value)
{
    for (;;)
    {
        int limit = _limit;
        if (limit + 4 <= _capacity)
        {
            _ptr[limit + 0] = (value >> (8 * 3)) & 0xff;
            _ptr[limit + 1] = (value >> (8 * 2)) & 0xff;
            _ptr[limit + 2] = (value >> (8 * 1)) & 0xff;
            _ptr[limit + 3] = (value >> (8 * 0)) & 0xff;
            if (InterlockedCompareExchange(&_limit, limit + 4, limit) == limit)
            {
                return *this;
            }
        }
        else
        {
            throw std::out_of_range("ByteBuffer::put");
        }
    }
}


ByteBuffer& ByteBuffer::put(signed int value)
{
    for (;;)
    {
        int limit = _limit;
        if (limit + 4 <= _capacity)
        {
            _ptr[limit + 0] = (value >> (8 * 3)) & 0xff;
            _ptr[limit + 1] = (value >> (8 * 2)) & 0xff;
            _ptr[limit + 2] = (value >> (8 * 1)) & 0xff;
            _ptr[limit + 3] = (value >> (8 * 0)) & 0xff;
            if (InterlockedCompareExchange(&_limit, limit + 4, limit) == limit)
            {
                return *this;
            }
        }
        else
        {
            throw std::out_of_range("ByteBuffer::put");
        }
    }
}


ByteBuffer& ByteBuffer::put(const void* value, size_t length)
{
    for (;;)
    {
        int limit = _limit;
        if (limit + static_cast<int>(length) <= _capacity)
        {
            memcpy(&_ptr[limit], value, length);
            if (InterlockedCompareExchange(&_limit, limit + static_cast<int>(length), limit) == limit)
            {
                return *this;
            }
        }
        else
        {
            throw std::out_of_range("ByteBuffer::put");
        }
    }
}


unsigned char ByteBuffer::peekU8(size_t offset) const
{
    int position = _position + static_cast<int>(offset);
    if (position + 1 <= _limit)
    {
        return _ptr[position];
    }
    else
    {
        throw std::out_of_range("ByteBuffer::peekU8");
    }
}


unsigned short ByteBuffer::peekU16(size_t offset) const
{
    int position = _position + static_cast<int>(offset);
    if (position + 2 <= _limit)
    {
        unsigned short value
            = ((unsigned short)_ptr[position + 0]) << (8 * 1)
            | ((unsigned short)_ptr[position + 1]) << (8 * 0);
        return value;
    }
    else
    {
        throw std::out_of_range("ByteBuffer::peekU16");
    }
}


unsigned int ByteBuffer::peekU32(size_t offset) const
{
    int position = _position + static_cast<int>(offset);
    if (position + 4 <= _limit)
    {
        unsigned int value
            = ((unsigned int)_ptr[position + 0]) << (8 * 3)
            | ((unsigned int)_ptr[position + 1]) << (8 * 2)
            | ((unsigned int)_ptr[position + 2]) << (8 * 1)
            | ((unsigned int)_ptr[position + 3]) << (8 * 0);
        return value;
    }
    else
    {
        throw std::out_of_range("ByteBuffer::peekU32");
    }
}


signed int ByteBuffer::peekS32(size_t offset) const
{
    int position = _position + static_cast<int>(offset);
    if (position + 4 <= _limit)
    {
        signed int value
            = ((signed int)_ptr[position + 0]) << (8 * 3)
            | ((signed int)_ptr[position + 1]) << (8 * 2)
            | ((signed int)_ptr[position + 2]) << (8 * 1)
            | ((signed int)_ptr[position + 3]) << (8 * 0);
        return value;
    }
    else
    {
        throw std::out_of_range("ByteBuffer::peekS32");
    }
}
