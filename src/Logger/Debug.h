// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_LOGGER_DEBUG_H
#define HNRT_LOGGER_DEBUG_H


#include "Logger.h"


namespace hnrt
{
    class Debug
    {
    public:

        Debug(const char* name);
        ~Debug();
        void put(const char* format, ...);

    private:

        Debug(const Debug&);
        void operator =(const Debug&);

        Logger& _log;
        const char* _name;
    };
}


#endif //!HNRT_LOGGER_DEBUG_H
