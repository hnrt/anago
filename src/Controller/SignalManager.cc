// Copyright (C) 2012-2018 Hideaki Narita


#include "SignalManagerImpl.h"


using namespace hnrt;


static SignalManagerImpl* _singleton = NULL;


void SignalManager::init()
{
    _singleton = new SignalManagerImpl();
}


void SignalManager::fini()
{
    delete _singleton;
}


SignalManager& SignalManager::instance()
{
    return *_singleton;
}
