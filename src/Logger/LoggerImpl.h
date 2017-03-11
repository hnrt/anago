// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_LOGGERIMPL_H
#define HNRT_LOGGERIMPL_H


#include <stdio.h>
#include <glibmm.h>
#include "Logger.h"


namespace hnrt
{
    class LoggerImpl
        : public Logger
    {
    public:

        LoggerImpl();
        ~LoggerImpl();
        virtual LogLevel getLevel() const { return _level; }
        virtual void setLevel(LogLevel level) { _level = level; }
        virtual void trace(const char*, ...);
        virtual void debug(const char*, ...);
        virtual void info(const char*, ...);
        virtual void warn(const char*, ...);
        virtual void error(const char*, ...);
        virtual void fatal(const char*, ...);

    private:

        LoggerImpl(const LoggerImpl&);
        void operator =(const LoggerImpl&);

        LogLevel _level;
        Glib::ustring _path;
        FILE* _fp;
        Glib::Mutex _mutex;
    };
}


#endif //!HNRT_LOGGERIMPL_H
