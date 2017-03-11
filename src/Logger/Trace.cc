// Copyright (C) 2012-2017 Hideaki Narita


#include "Logger.h"
#include "Trace.h"


using namespace hnrt;


Trace::Trace(const char* name)
    : _log(Logger::instance())
    , _name(name)
{
    _log.trace("%s: Started.", _name);
}


Trace::~Trace()
{
    _log.trace("%s: Finished.", _name);
}


void Trace::put(const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    _log.trace2(_name, format, argList);
    va_end(argList);
}
