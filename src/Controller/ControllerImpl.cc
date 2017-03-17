// Copyright (C) 2012-2017 Hideaki Narita


#include <stdexcept>
#include "Base/Atomic.h"
#include "Logger/Trace.h"
#include "View/View.h"
#include "ControllerImpl.h"


using namespace hnrt;


ControllerImpl::ControllerImpl()
    : _backgroundCount(0)
    , _quitInProgress(false)
{
    Trace trace("ControllerImpl::ctor");
}


ControllerImpl::~ControllerImpl()
{
    Trace trace("ControllerImpl::dtor");
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
            Logger::instance().setLevel(LogLevel::parse(argv[index]));
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
    Trace trace(__PRETTY_FUNCTION__);

    if (!_quitInProgress)
    {
        _quitInProgress = true;
        View::instance().getWindow().set_title("Quitting..."); //TODO: LOCALIZE
        if (quit1())
        {
            Glib::signal_timeout().connect(sigc::mem_fun(*this, &ControllerImpl::quit1), 100); // 100 milleseconds
        }
    }
}


bool ControllerImpl::quit1()
{
    Trace trace(__PRETTY_FUNCTION__);

    int busyCount = 0;
    //TODO: DISCONNECT SESSIONS
    if (busyCount || _backgroundCount)
    {
        trace.put("busy=%d background=%d", busyCount, _backgroundCount);
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


void ControllerImpl::notify(RefPtr<RefObj> object, Notification notif)
{
    //TODO: IMPLEMENT
}


void ControllerImpl::addHost()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::editHost()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::removeHost()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::connect()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::disconnect()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::showAbout()
{
    //TODO: IMPLEMENT
}
