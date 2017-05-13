// Copyright (C) 2012-2017 Hideaki Narita


#include <string.h>
#include "LogLevel.h"


using namespace hnrt;


static const char TEXT_DEFAULT[] = { "DEFAULT" };
static const char TEXT_TRACE[] = { "TRACE" };
static const char TEXT_DEBUG[] = { "DEBUG" };
static const char TEXT_INFO[] = { "INFO" };
static const char TEXT_WARNING[] = { "WARNING" };
static const char TEXT_ERROR[] = { "ERROR" };
static const char TEXT_FATAL[] = { "FATAL" };
static const char TEXT_UNDEFINED[] = { "UNDEFINED" };


LogLevel LogLevel::parse(const char* value)
{
    if (!value || !*value || !strcasecmp(value, TEXT_DEFAULT))
    {
        return LogLevel::INFO;
    }
    else if (!strcasecmp(value, TEXT_TRACE))
    {
        return LogLevel::TRACE;
    }
    else if (!strcasecmp(value, TEXT_DEBUG))
    {
        return LogLevel::DEBUG;
    }
    else if (!strcasecmp(value, TEXT_INFO))
    {
        return LogLevel::INFO;
    }
    else if (!strcasecmp(value, TEXT_WARNING))
    {
        return LogLevel::WARNING;
    }
    else if (!strcasecmp(value, TEXT_ERROR))
    {
        return LogLevel::ERROR;
    }
    else if (!strcasecmp(value, TEXT_FATAL))
    {
        return LogLevel::FATAL;
    }
    else
    {
        return LogLevel::UNDEFINED;
    }
}


const char* LogLevel::toString() const
{
    if (_value == LogLevel::TRACE)
    {
        return TEXT_TRACE;
    }
    else if (_value == LogLevel::DEBUG)
    {
        return TEXT_DEBUG;
    }
    else if (_value == LogLevel::INFO)
    {
        return TEXT_INFO;
    }
    else if (_value == LogLevel::WARNING)
    {
        return TEXT_WARNING;
    }
    else if (_value == LogLevel::ERROR)
    {
        return TEXT_ERROR;
    }
    else if (_value == LogLevel::FATAL)
    {
        return TEXT_FATAL;
    }
    else
    {
        return TEXT_UNDEFINED;
    }
}


const char* LogLevel::toLocalizedString() const
{
    return toString(); //TODO: Localize!
}
