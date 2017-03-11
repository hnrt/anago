// Copyright (C) 2012-2017 Hideaki Narita


#include "ControllerImpl.h"


using namespace hnrt;


static ControllerImpl* _singleton = NULL;


void Controller::init()
{
    _singleton = new ControllerImpl();
}


void Controller::fini()
{
    delete _singleton;
}


Controller& Controller::instance()
{
    return *_singleton;
}
