// Copyright (C) 2012-2017 Hideaki Narita


#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include "ByteString.h"


namespace hnrt
{
    struct ByteStringHeader
    {
        size_t length;
        size_t extraLength;
    };
}


using namespace hnrt;


ByteString::ByteString()
    : _value(NULL)
{
}


ByteString::ByteString(const void* p, size_t n, size_t extra)
    : _value(NULL)
{
    _assign(p, n, extra);
}


ByteString::ByteString(const ByteString& src)
    : _value(NULL)
{
    _assign(src._value, src.getLength(), src.getExtraLength());
}


ByteString::~ByteString()
{
    _free();
}


size_t ByteString::getLength() const
{
    return _value ? reinterpret_cast<ByteStringHeader*>(_value)[-1].length : 0;
}


size_t ByteString::getExtraLength() const
{
    return _value ? reinterpret_cast<ByteStringHeader*>(_value)[-1].extraLength : 0;
}


ByteString& ByteString::clear()
{
    _free();
    _value = NULL;
    return *this;
}


ByteString& ByteString::assign(const void* p, size_t n, size_t extra)
{
    _free();
    _value = NULL;
    _assign(p, n, extra);
    return *this;
}


ByteString& ByteString::assign(const ByteString& src)
{
    _free();
    _value = NULL;
    _assign(src._value, src.getLength(), src.getExtraLength());
    return *this;
}


int ByteString::compare(const void* p, size_t n)
{
    int result;
    size_t length = getLength();
    if (length < n)
    {
        if (!length || !(result = memcmp(_value, p, length)))
        {
            result = -1;
        }
    }
    else if (n < length)
    {
        if (!n || !(result = memcmp(_value, p, n)))
        {
            result = 1;
        }
    }
    else
    {
        result = memcmp(_value, p, length);
    }
    return result;
}


void ByteString::_free()
{
    if (_value)
    {
        ByteStringHeader* h = reinterpret_cast<ByteStringHeader*>(_value) - 1;
        memset(_value, 0, h->length + h->extraLength);
        free(h);
    }
}


void ByteString::_assign(const void *p, size_t n, size_t extra)
{
    size_t totalLength = n + extra;
    if (totalLength > 0)
    {
        ByteStringHeader* h = reinterpret_cast<ByteStringHeader*>(malloc(sizeof(ByteStringHeader) + totalLength));
        if (!h)
        {
            throw std::bad_alloc();
        }
        h->length = n;
        h->extraLength = extra;
        _value = reinterpret_cast<unsigned char*>(h + 1);
        if (p && n)
        {
            memcpy(_value, p, n);
            if (extra)
            {
                memset(_value + n, 0, extra);
            }
        }
        else
        {
            memset(_value, 0, totalLength);
        }
    }
}


void ByteString::_setLength(size_t n)
{
    if (_value)
    {
        ByteStringHeader* h = reinterpret_cast<ByteStringHeader*>(_value) - 1;
        size_t totalLength = h->length + h->extraLength;
        if (n <= totalLength)
        {
            h->length = n;
            h->extraLength = totalLength - n;
            return;
        }
    }
    throw std::runtime_error("ByteString::_setLength: Too big value!");
}
