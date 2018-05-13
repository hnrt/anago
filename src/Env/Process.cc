// Copyright (C) 2012-2018 Hideaki Narita


#include "ProcessImpl.h"


using namespace hnrt;


static ProcessImpl* _singleton = NULL;


void Process::init()
{
    _singleton = new ProcessImpl();
}


void Process::fini()
{
    delete _singleton;
}


Process& Process::instance()
{
    return *_singleton;
}
