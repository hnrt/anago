// Copyright (C) 2017 Hideaki Narita


#include "ThreadManagerImpl.h"


using namespace hnrt;


static ThreadManagerImpl* _singleton;


void ThreadManager::init()
{
    _singleton = new ThreadManagerImpl();
}


void ThreadManager::fini()
{
    delete _singleton;
}


ThreadManager& ThreadManager::instance()
{
    return *_singleton;
}
