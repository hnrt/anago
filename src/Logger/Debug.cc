// Copyright (C) 2012-2018 Hideaki Narita


#include "Logger.h"
#include "Debug.h"


using namespace hnrt;


Debug::Debug(const char* name)
    : _log(Logger::instance())
    , _name(name)
{
    _log.debug("%s: Started.", _name);
}


Debug::~Debug()
{
    _log.debug("%s: Finished.", _name);
}


void Debug::put(const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    _log.debug2(_name, format, argList);
    va_end(argList);
}
