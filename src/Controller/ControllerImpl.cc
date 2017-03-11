// Copyright (C) 2012-2017 Hideaki Narita


#include <stdexcept>
#include "View/View.h"
#include "ControllerImpl.h"


using namespace hnrt;


ControllerImpl::ControllerImpl()
{
}


ControllerImpl::~ControllerImpl()
{
}


void ControllerImpl::parseCommandLine(int argc, char *argv[])
{
    for (int index = 1; index < argc; index++)
    {
        {
            static char msg[256];
            snprintf(msg, sizeof(msg), "Bad command line syntax: %s", argv[index]);
            throw std::runtime_error(msg);
        }
    }
}


void ControllerImpl::quit()
{
    View::instance().getWindow().hide();
}
