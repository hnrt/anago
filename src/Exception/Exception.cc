// Copyright (C) 2012-2018 Hideaki Narita


#include "Base/StringBuffer.h"
#include "Exception.h"


using namespace hnrt;


Exception::Exception()
{
}


Exception::Exception(const Exception& other)
    : _what(other._what)
{
}


Exception::Exception(const Glib::ustring& s)
    : _what(s)
{
}


Exception::Exception(const char* format, ...)
{
    va_list v;
    va_start(v, format);
    assignV(format, v);
    va_end(v);
}


Exception::~Exception()
{
}


Exception& Exception::assign(const Exception& other)
{
    _what = other._what;
    return *this;
}


Exception& Exception::assignV(const char* format, va_list v)
{
    _what = StringBuffer().formatV(format, v).str();
    return *this;
}


Exception& Exception::assign(const char* format, ...)
{
    va_list v;
    va_start(v, format);
    assignV(format, v);
    va_end(v);
    return *this;
}
