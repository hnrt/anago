// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_LOGGER_TRACE_H
#define HNRT_LOGGER_TRACE_H


#include "Logger.h"


namespace hnrt
{
    class Trace
    {
    public:

        Trace(const char* name);
        ~Trace();
        void put(const char* format, ...);

    private:

        Trace(const Trace&);
        void operator =(const Trace&);

        Logger& _log;
        const char* _name;
    };
}


#endif //!HNRT_LOGGER_TRACE_H
