// Copyright (C) 2012-2017 Hideaki Narita


#include "Base/StringBuffer.h"
#include "Logger.h"
#include "Trace.h"


using namespace hnrt;


Trace::Trace(const char* name)
    : _log(Logger::instance())
    , _name(name)
{
    _log.trace("%s: Started.", _name.c_str());
}


Trace::Trace(const char* name, const char* format, ...)
    : _log(Logger::instance())
    , _name(name)
{
    StringBuffer buf;
    buf.format("%s: Started: ", _name.c_str());
    va_list argList;
    va_start(argList, format);
    buf.appendFormatV(format, argList);
    va_end(argList);
    _log.trace("%s", buf.str());
}


Trace::~Trace()
{
    _log.trace("%s: Finished.", _name.c_str());
}


void Trace::put(const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    _log.trace2(_name.c_str(), format, argList);
    va_end(argList);
}
