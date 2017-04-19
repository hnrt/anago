// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONSOLEINFO_H
#define HNRT_CONSOLEINFO_H


#include <glibmm/ustring.h>
#include <map>


namespace hnrt
{
    struct ConsoleInfo
    {
        Glib::ustring uuid;
        bool enabled;
        bool scale;

        inline ConsoleInfo(const Glib::ustring);
        inline ConsoleInfo(const ConsoleInfo&);
        inline void operator =(const ConsoleInfo&);
        inline void assign(const ConsoleInfo&);
    };

    typedef std::map<Glib::ustring, ConsoleInfo> ConsoleMap;
    typedef std::pair<Glib::ustring, ConsoleInfo> ConsoleEntry;

    inline ConsoleInfo::ConsoleInfo(const Glib::ustring uuid_)
        : uuid(uuid_)
        , enabled(true)
        , scale(false)
    {
    }

    inline ConsoleInfo::ConsoleInfo(const ConsoleInfo& src)
    {
        assign(src);
    }

    inline void ConsoleInfo::operator =(const ConsoleInfo& rhs)
    {
        assign(rhs);
    }

    inline void ConsoleInfo::assign(const ConsoleInfo& rhs)
    {
        uuid = rhs.uuid;
        enabled = rhs.enabled;
        scale = rhs.scale;
    }
}


#endif //!HNRT_CONSOLEINFO_H
