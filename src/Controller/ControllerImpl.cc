// Copyright (C) 2012-2017 Hideaki Narita


#include <stdexcept>
#include "View/View.h"
#include "ControllerImpl.h"


using namespace hnrt;


ControllerImpl::ControllerImpl()
    : _log(Logger::instance())
{
    _log.trace("ControllerImpl::ctor");
}


ControllerImpl::~ControllerImpl()
{
    _log.trace("ControllerImpl::dtor");
}


void ControllerImpl::parseCommandLine(int argc, char *argv[])
{
    for (int index = 1; index < argc; index++)
    {
        if (!strcmp(argv[index], "-log"))
        {
            if (++index == argc || argv[index][0] == '-')
            {
                static char msg[256];
                snprintf(msg, sizeof(msg), "Command line option -log operand was not specified.");
                throw std::runtime_error(msg);
            }
            _log.setLevel(LogLevel::parse(argv[index]));
        }
        else
        {
            static char msg[256];
            snprintf(msg, sizeof(msg), "Bad command line syntax: %s", argv[index]);
            throw std::runtime_error(msg);
        }
    }
}


void ControllerImpl::quit()
{
    _log.trace("ControllerImpl::quit: Entered.");

    View::instance().getWindow().hide();

    _log.trace("ControllerImpl::quit: Finished.");
}
