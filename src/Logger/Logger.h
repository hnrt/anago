// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_LOGGER_H
#define HNRT_LOGGER_H


#include <stdarg.h>
#include "LogLevel.h"


namespace hnrt
{
    class Logger
    {
    public:

        static void init();
        static void fini();
        static Logger& instance();

        virtual LogLevel getLevel() const = 0;
        virtual void setLevel(LogLevel) = 0;
        virtual void trace(const char*, ...) = 0;
        virtual void trace2(const char*, const char*, va_list) = 0;
        virtual void debug(const char*, ...) = 0;
        virtual void debug2(const char*, const char*, va_list) = 0;
        virtual void info(const char*, ...) = 0;
        virtual void warn(const char*, ...) = 0;
        virtual void error(const char*, ...) = 0;
        virtual void fatal(const char*, ...) = 0;
    };
}


#endif //!HNRT_LOGGER_H
