// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_CONSOLE_EXCEPTION_H
#define HNRT_CONSOLE_EXCEPTION_H


#include "Exception.h"


namespace hnrt
{
    class ConsoleException
        : public Exception
    {
    public:

        ConsoleException();
        ConsoleException(const ConsoleException&);
        ConsoleException(const Glib::ustring&);
        ConsoleException(const char*, ...);
        ConsoleException& operator =(const ConsoleException& rhs) { return static_cast<ConsoleException&>(assign(rhs)); }
    };

    class LocationConsoleException
        : public ConsoleException
    {
    public:

        LocationConsoleException();
        LocationConsoleException(const LocationConsoleException&);
        LocationConsoleException(const Glib::ustring&);
        LocationConsoleException(const char*, ...);
        LocationConsoleException& operator =(const LocationConsoleException& rhs) { return static_cast<LocationConsoleException&>(assign(rhs)); }
    };

    class CommunicationConsoleException
        : public ConsoleException
    {
    public:

        CommunicationConsoleException(int);
        CommunicationConsoleException(const CommunicationConsoleException&);
        CommunicationConsoleException(int, const Glib::ustring&);
        CommunicationConsoleException(int, const char*, ...);
        CommunicationConsoleException& assign(const CommunicationConsoleException& rhs);
        CommunicationConsoleException& operator =(const CommunicationConsoleException& rhs) { return assign(rhs); }
        int error() const { return _error; }

    protected:

        int _error;
    };
}


#endif //!HNRT_CONSOLE_EXCEPTION_H
