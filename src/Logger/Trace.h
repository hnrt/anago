// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_LOGGER_TRACE_H
#define HNRT_LOGGER_TRACE_H


#include <glibmm.h>
#include "Logger.h"


namespace hnrt
{
    class Trace
    {
    public:

        Trace(const char*);
        Trace(const char*, const char*, ...);
        ~Trace();
        void put(const char*, ...);

    private:

        Trace(const Trace&);
        void operator =(const Trace&);

        Logger& _log;
        Glib::ustring _name;
    };
}


#ifdef _DEBUG
#define TRACE(fun,...) Trace trace__(fun,##__VA_ARGS__)
#define TRACEPUT(fmt,...) trace__.put(fmt,##__VA_ARGS__)
#else
#define TRACE(fun,...) (void)0
#define TRACEPUT(fmt,...) (void)0
#endif


#endif //!HNRT_LOGGER_TRACE_H
