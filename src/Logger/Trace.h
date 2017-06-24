// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_LOGGER_TRACE_H
#define HNRT_LOGGER_TRACE_H


#include <glibmm.h>
#include "Logger.h"


namespace hnrt
{
    class XenObject;

    class Trace
    {
    public:

        Trace(const void*, const char*, ...);
        ~Trace();
        void put(const char*, ...);
        const Glib::ustring& name() const { return _name; }

    private:

        Trace(const Trace&);
        void operator =(const Trace&);

        Logger& _log;
        Glib::ustring _name;
    };

    struct XenObjectText
    {
        char* ptr;

        XenObjectText(const XenObject&);
        ~XenObjectText();
        operator const char*() const { return ptr; }

    private:

        XenObjectText(const XenObjectText&);
        void operator =(const XenObjectText&);
    };
}


#if defined(_DEBUG) && !defined(NO_TRACE)
#define TRACEFUN(ptr,fmt,...) Trace trace__(ptr,fmt,##__VA_ARGS__)
#define TRACEPUT(fmt,...) trace__.put(fmt,##__VA_ARGS__)
#define TRACE(fmt,...) hnrt::Logger::instance().trace(fmt,##__VA_ARGS__)
#else
#define TRACEFUN(ptr,fmt,...) (void)0
#define TRACEPUT(fmt,...) (void)0
#define TRACE(fmt,...) (void)0
#endif


#endif //!HNRT_LOGGER_TRACE_H
