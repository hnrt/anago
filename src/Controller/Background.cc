// Copyright (C) 2012-2017 Hideaki Narita


#include "Background.h"
#include "Controller.h"


using namespace hnrt;


Background::Background()
{
    Controller::instance().incBackgroundCount();
}


Background::~Background()
{
    Controller::instance().decBackgroundCount();
}
