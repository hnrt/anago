// Copyright (C) 2012-2017 Hideaki Narita


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

        CommunicationConsoleException();
        CommunicationConsoleException(const CommunicationConsoleException&);
        CommunicationConsoleException(const Glib::ustring&);
        CommunicationConsoleException(const char*, ...);
        CommunicationConsoleException& operator =(const CommunicationConsoleException& rhs) { return static_cast<CommunicationConsoleException&>(assign(rhs)); }
    };
}


#endif //!HNRT_CONSOLE_EXCEPTION_H
