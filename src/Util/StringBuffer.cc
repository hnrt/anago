// Copyright (C) 2012-2017 Hideaki Narita


#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include "StringBuffer.h"


using namespace hnrt;


StringBuffer::StringBuffer(size_t n)
    : _ptr(NULL)
    , _capacity(0)
    , _length(0)
{
    expand(n > 64 ? n : 64);
}


StringBuffer::StringBuffer(const StringBuffer& src)
    : _ptr(NULL)
    , _capacity(0)
    , _length(0)
{
    assign(src);
}


StringBuffer::~StringBuffer()
{
    free();
}


char& StringBuffer::charAt(size_t index) const
{
    if (index <= _length && _capacity)
    {
        return _ptr[index];
    }
    else
    {
        throw std::out_of_range("StringBuffer::charAt");
    }
}


StringBuffer& StringBuffer::resize(size_t n)
{
    if (n > _capacity)
    {
        char* newBuf = new char[n];
        memcpy(newBuf, _ptr, _capacity);
        memset(newBuf + _capacity, 0, n - _capacity);
        memset(_ptr, 0, _length);
        delete[] _ptr;
        _ptr = newBuf;
        _capacity = n;
    }
    else if (n == _capacity)
    {
        // nothing to do
    }
    else if (n)
    {
        setLength(n - 1);
        _capacity = n;
    }
    else
    {
        memset(_ptr, 0, _length);
        delete[] _ptr;
        _ptr = NULL;
        _length = 0;
        _capacity = 0;
    }
    return *this;
}


StringBuffer& StringBuffer::setLength(size_t n)
{
    if (n < _length)
    {
        memset(_ptr + n, 0, _length - n);
        _length = n;
    }
    return *this;
}


StringBuffer& StringBuffer::assign(const StringBuffer& src)
{
    free();
    if (src._capacity)
    {
        _ptr = new char[src._capacity];
        memcpy(_ptr, src._ptr, src._length);
        memset(_ptr + src._length, 0, src._capacity - src._length);
        _capacity = src._capacity;
        _length = src._length;
    }
    return *this;
}


StringBuffer& StringBuffer::assign(char c)
{
    clear();
    return append(c);
}


StringBuffer& StringBuffer::assign(const char* s, ssize_t n)
{
    clear();
    return append(s, n);
}


StringBuffer &StringBuffer::format(const char* s, ...)
{
    clear();
    va_list v;
    va_start(v, s);
    appendFormatV(s, v);
    va_end(v);
    return *this;
}


StringBuffer& StringBuffer::formatV(const char* s, va_list v)
{
    clear();
    return appendFormatV(s, v);
}


StringBuffer& StringBuffer::append(const StringBuffer& src)
{
    return append(src._ptr, src._length);
}


StringBuffer& StringBuffer::append(char c)
{
    if (_capacity < _length + 2)
    {
        expand(_length + 2);
    }
    _ptr[_length++] = c;
    return *this;
}


StringBuffer& StringBuffer::append(const char* s, ssize_t n)
{
    if (!s)
    {
        return *this;
    }
    if (n < 0)
    {
        n = strlen(s);
    }
    if (!n)
    {
        return *this;
    }
    if (_capacity < _length + n + 1)
    {
        expand(_length + n + 1);
    }
    memcpy(&_ptr[_length], s, n);
    _length += n;
    return *this;
}


StringBuffer& StringBuffer::appendFormat(const char* s, ...)
{
    va_list v;
    va_start(v, s);
    appendFormatV(s, v);
    va_end(v);
    return *this;
}


StringBuffer& StringBuffer::appendFormatV(const char* s, va_list v)
{
    int n = 32;
    while (n > 0)
    {
        expand(_length + n);
        va_list v2;
        va_copy(v2, v);
        int m = vsnprintf(&_ptr[_length], n, s, v2);
        va_end(v2);
        if (m < 0)
        {
            //n *= 2;
            fprintf(stderr, "Error: StringBuffer::appendFormatV(%s): vsnprintf returned %d\n", s, m);
            return *this;
        }
        else if (n <= m)
        {
            n = m + 1;
        }
        else
        {
            _length += m;
            memset(&_ptr[_length], 0, _capacity - _length);
            break;
        }
    }
    if (n < 0)
    {
        fprintf(stderr, "Error: StringBuffer::appendFormatV: n=%d s=(%s)\n", n, s);
    }
    return *this;
}


void StringBuffer::expand(size_t required)
{
    if (_capacity < required)
    {
        size_t newCap = _capacity;
        do
        {
            newCap += !newCap ? 64 : (newCap < 65536 ? newCap : 65536);
        }
        while (newCap < required);
        char* newBuf = new char[newCap];
        if (_capacity)
        {
            if (_length)
            {
                memcpy(newBuf, _ptr, _length);
                memset(_ptr, 0, _length);
            }
            delete[] _ptr;
        }
        memset(newBuf + _length, 0, newCap - _length);
        _ptr = newBuf;
        _capacity = newCap;
    }
}


void StringBuffer::free()
{
    if (_capacity)
    {
        if (_length)
        {
            memset(_ptr, 0, _length);
            _length = 0;
        }
        delete[] _ptr;
        _ptr = NULL;
        _capacity = 0;
    }
}
