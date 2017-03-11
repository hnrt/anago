// Copyright (C) 2012-2017 Hideaki Narita


#include <stdexcept>
#include "Base/Atomic.h"
#include "View/View.h"
#include "ControllerImpl.h"


using namespace hnrt;


ControllerImpl::ControllerImpl()
    : _log(Logger::instance())
    , _backgroundCount(0)
    , _quitInProgress(false)
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

    if (!_quitInProgress)
    {
        _quitInProgress = true;
        View::instance().getWindow().set_title("Quitting..."); //TODO: LOCALIZE
        if (quit1())
        {
            Glib::signal_timeout().connect(sigc::mem_fun(*this, &ControllerImpl::quit1), 100); // 100 milleseconds
        }
    }

    _log.trace("ControllerImpl::quit: Finished.");
}


bool ControllerImpl::quit1()
{
    int busyCount = 0;
    //TODO: DISCONNECT SESSIONS
    if (busyCount || _backgroundCount)
    {
        _log.trace("ControllerImpl::quit1: busy=%d background=%d", busyCount, _backgroundCount);
        return true; // to be invoked again
    }
    else
    {
        View::instance().getWindow().hide();
        return false; // done
    }
}


void ControllerImpl::incBackgroundCount()
{
    InterlockedIncrement(&_backgroundCount);
}


void ControllerImpl::decBackgroundCount()
{
    InterlockedDecrement(&_backgroundCount);
}
