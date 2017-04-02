// Copyright (C) 2012-2017 Hideaki Narita


#include "Base/StringBuffer.h"
#include "ConsoleException.h"


using namespace hnrt;


ConsoleException::ConsoleException()
{
}


ConsoleException::ConsoleException(const ConsoleException& other)
    : Exception(other)
{
}


ConsoleException::ConsoleException(const Glib::ustring& s)
    : Exception(s)
{
}


ConsoleException::ConsoleException(const char* format, ...)
{
    va_list v;
    va_start(v, format);
    assignV(format, v);
    va_end(v);
}


LocationConsoleException::LocationConsoleException()
{
}


LocationConsoleException::LocationConsoleException(const LocationConsoleException& other)
    : ConsoleException(other)
{
}


LocationConsoleException::LocationConsoleException(const Glib::ustring& s)
    : ConsoleException(s)
{
}


LocationConsoleException::LocationConsoleException(const char* format, ...)
{
    va_list v;
    va_start(v, format);
    assignV(format, v);
    va_end(v);
}


CommunicationConsoleException::CommunicationConsoleException(int error)
    : _error(error)
{
}


CommunicationConsoleException::CommunicationConsoleException(const CommunicationConsoleException& other)
    : ConsoleException(other)
    , _error(other._error)
{
}


CommunicationConsoleException::CommunicationConsoleException(int error, const Glib::ustring& s)
    : ConsoleException(s)
    , _error(error)
{
}


CommunicationConsoleException::CommunicationConsoleException(int error, const char* format, ...)
    : _error(error)
{
    va_list v;
    va_start(v, format);
    assignV(format, v);
    va_end(v);
}


CommunicationConsoleException& CommunicationConsoleException::assign(const CommunicationConsoleException& rhs)
{
    Exception::assign(rhs);
    _error = rhs._error;
    return *this;
}
