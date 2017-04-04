// Copyright (C) 2017 Hideaki Narita


#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include "ByteBuffer.h"


using namespace hnrt;


RefPtr<ByteBuffer> ByteBuffer::create(size_t capacity)
{
    return RefPtr<ByteBuffer>(new ByteBuffer(capacity));
}


RefPtr<ByteBuffer> ByteBuffer::create(const void* value, size_t length)
{
    return RefPtr<ByteBuffer>(new ByteBuffer(value, length));
}


ByteBuffer::ByteBuffer(size_t capacity)
    : _ptr(NULL)
    , _capacity(0)
    , _limit(0)
    , _position(0)
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
    }
}


ByteBuffer::~ByteBuffer()
{
    free(_ptr);
}


ByteBuffer& ByteBuffer::capacity(size_t requested)
{
    if (requested != _capacity)
    {
        if (requested)
        {
            unsigned char* ptr = (unsigned char*)realloc(_ptr, requested);
            if (!ptr)
            {
                throw std::bad_alloc();
            }
            _ptr = ptr;
            _capacity = requested;
            if (_limit > _capacity)
            {
                _limit = _capacity;
                if (_position > _limit)
                {
                    _position = _limit;
                }
            }
        }
        else
        {
            free(_ptr);
            _ptr = NULL;
            _capacity = 0;
            _limit = 0;
            _position = 0;
        }
    }
    return *this;
}


ByteBuffer& ByteBuffer::limit(size_t requested)
{
    if (requested > _capacity)
    {
        throw std::invalid_argument("ByteBuffer::limit: Out of bounds.");
    }
    _limit = requested;
    if (_position > _limit)
    {
        _position = _limit;
    }
    return *this;
}


ByteBuffer& ByteBuffer::position(size_t requested)
{
    if (requested > _limit)
    {
        throw std::invalid_argument("ByteBuffer::position: Out of bounds.");
    }
    _position = requested;
    return *this;
}


ByteBuffer& ByteBuffer::clear()
{
    _limit = _capacity;
    _position = 0;
    return *this;
}


ByteBuffer& ByteBuffer::flip()
{
    _limit = _position;
    _position = 0;
    return *this;
}


ByteBuffer& ByteBuffer::rewind()
{
    _position = 0;
    return *this;
}


ByteBuffer& ByteBuffer::prepareForRead()
{
    if (_position)
    {
        size_t remaining = _limit - _position;
        memmove(_ptr, _ptr + _position, remaining);
        _position = 0;
        _limit = remaining;
    }
    _position = _limit;
    _limit = _capacity;
    return *this;
}


ByteBuffer& ByteBuffer::set(const void* value, size_t length)
{
    if (length)
    {
        if (length != _capacity)
        {
            unsigned char* ptr = (unsigned char*)realloc(_ptr, length);
            if (!ptr)
            {
                throw std::bad_alloc();
            }
            _ptr = ptr;
            _capacity = length;
        }
        if (value)
        {
            memcpy(_ptr, value, length);
        }
        else
        {
            memset(_ptr, 0, length);
        }
        _limit = _capacity;
        _position = 0;
    }
    else if (_capacity)
    {
        free(_ptr);
        _ptr = NULL;
        _capacity = 0;
        _limit = 0;
        _position = 0;
    }
    return *this;
}


const unsigned char& ByteBuffer::operator [](size_t index) const
{
    if (index < _limit)
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
    if (index < _limit)
    {
        return _ptr[index];
    }
    else
    {
        throw std::out_of_range("ByteBuffer::operator[]");
    }
}


unsigned char ByteBuffer::get()
{
    if (_position < _limit)
    {
        return _ptr[_position++];
    }
    else
    {
        throw std::out_of_range("ByteBuffer::get");
    }
}


ByteBuffer& ByteBuffer::put(unsigned char value)
{
    if (_position + 1 <= _limit)
    {
        _ptr[_position++] = value;
        return *this;
    }
    else
    {
        throw std::out_of_range("ByteBuffer::put");
    }
}


ByteBuffer& ByteBuffer::put(unsigned short value)
{
    if (_position + 2 <= _limit)
    {
        _ptr[_position++] = (value >> (8 * 1)) & 0xff;
        _ptr[_position++] = (value >> (8 * 0)) & 0xff;
        return *this;
    }
    else
    {
        throw std::out_of_range("ByteBuffer::put");
    }
}


ByteBuffer& ByteBuffer::put(unsigned int value)
{
    if (_position + 4 <= _limit)
    {
        _ptr[_position++] = (value >> (8 * 3)) & 0xff;
        _ptr[_position++] = (value >> (8 * 2)) & 0xff;
        _ptr[_position++] = (value >> (8 * 1)) & 0xff;
        _ptr[_position++] = (value >> (8 * 0)) & 0xff;
        return *this;
    }
    else
    {
        throw std::out_of_range("ByteBuffer::put");
    }
}


ByteBuffer& ByteBuffer::put(signed int value)
{
    if (_position + 4 <= _limit)
    {
        _ptr[_position++] = (value >> (8 * 3)) & 0xff;
        _ptr[_position++] = (value >> (8 * 2)) & 0xff;
        _ptr[_position++] = (value >> (8 * 1)) & 0xff;
        _ptr[_position++] = (value >> (8 * 0)) & 0xff;
        return *this;
    }
    else
    {
        throw std::out_of_range("ByteBuffer::put");
    }
}


ByteBuffer& ByteBuffer::put(const void* value, size_t length)
{
    if (_position + length <= _limit)
    {
        memcpy(&_ptr[_position], value, length);
        _position += length;
        return *this;
    }
    else
    {
        throw std::out_of_range("ByteBuffer::put");
    }
}
