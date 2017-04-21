// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONSOLEINFO_H
#define HNRT_CONSOLEINFO_H


#include <glibmm/ustring.h>
#include <map>
#include "Base/RefPtr.h"


namespace hnrt
{
    class Console;

    struct ConsoleInfo
    {
        Glib::ustring uuid;
        bool enabled;
        bool scale;
        RefPtr<Console> console;

        ConsoleInfo(const Glib::ustring&);
        ConsoleInfo(const ConsoleInfo&);
        inline void operator =(const ConsoleInfo&);
        void assign(const ConsoleInfo&);
    };

    typedef std::map<Glib::ustring, ConsoleInfo> ConsoleMap;
    typedef std::pair<Glib::ustring, ConsoleInfo> ConsoleEntry;

    inline void ConsoleInfo::operator =(const ConsoleInfo& rhs)
    {
        assign(rhs);
    }
}


#endif //!HNRT_CONSOLEINFO_H
