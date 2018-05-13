// Copyright (C) 2012-2018 Hideaki Narita


#include "XenServer/HardDiskDriveSpec.h"
#include "ControllerImpl.h"
#include "SignalManager.h"


using namespace hnrt;


static ControllerImpl* _singleton = NULL;


void Controller::init()
{
    SignalManager::init();

    _singleton = new ControllerImpl();
}


void Controller::fini()
{
    delete _singleton;

    SignalManager::fini();
}


Controller& Controller::instance()
{
    return *_singleton;
}
