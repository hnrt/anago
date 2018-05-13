// Copyright (C) 2012-2018 Hideaki Narita


#include "LoggerImpl.h"


using namespace hnrt;


static LoggerImpl* _singleton = NULL;


void Logger::init()
{
    _singleton = new LoggerImpl();
}


void Logger::fini()
{
    delete _singleton;
}


Logger& Logger::instance()
{
    return *_singleton;
}
