// Copyright (C) 2012-2017 Hideaki Narita


#include "View/View.h"
#include "ControllerImpl.h"


using namespace hnrt;


ControllerImpl::ControllerImpl()
{
}


ControllerImpl::~ControllerImpl()
{
}


void ControllerImpl::quit()
{
    View::instance().getWindow().hide();
}
