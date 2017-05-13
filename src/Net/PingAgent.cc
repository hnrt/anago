// Copyright (C) 2012-2017 Hideaki Narita


#include "PingAgentImpl.h"


using namespace hnrt;


static PingAgentImpl* _singleton;


void PingAgent::init()
{
    _singleton = new PingAgentImpl();
}


void PingAgent::fini()
{
    delete _singleton;
}


PingAgent& PingAgent::instance()
{
    return *_singleton;
}
