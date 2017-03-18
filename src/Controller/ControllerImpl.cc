// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <stdexcept>
#include "Base/Atomic.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "View/View.h"
#include "XenServer/Host.h"
#include "XenServer/Session.h"
#include "XenServer/XenObject.h"
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


void ControllerImpl::clear()
{
    Trace trace("ControllerImpl::clear");
    Glib::RecMutex::Lock k(_mutex);
    _notified.clear();
    _notificationSignalMap.clear();
    _refObjSignalMap.clear();
}


void ControllerImpl::parseCommandLine(int argc, char *argv[])
{
    Trace trace("ControllerImpl::parseCommandLine");

    _dispatcher.connect(sigc::mem_fun(*this, &ControllerImpl::onNotify));

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
    Trace trace("ControllerImpl::quit");

    if (!_quitInProgress)
    {
        _quitInProgress = true;
        View::instance().getWindow().set_title(gettext("Quitting..."));
        if (quit2())
        {
            Glib::signal_timeout().connect(sigc::mem_fun(*this, &ControllerImpl::quit2), 100); // 100 milleseconds
        }
    }
}


bool ControllerImpl::quit2()
{
    Trace trace("ControllerImpl::quit2");

    int busyCount = 0;
    std::list<RefPtr<Host> > hosts;
    Model::instance().get(hosts);
    for (std::list<RefPtr<Host> >::iterator iter = hosts.begin(); iter != hosts.end(); iter++)
    {
        RefPtr<Host>& host = *iter;
        if (host->isBusy())
        {
            busyCount++;
        }
        else
        {
            Session& session = host->getSession();
            if (session.isConnected())
            {
                if (session.disconnect())
                {
                    host->onDisconnected();
                }
            }
        }
    }
    hosts.clear();
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


Controller::Signal ControllerImpl::signalNotified(int notification)
{
    Glib::RecMutex::Lock k(_mutex);
    NotificationSignalMap::iterator iter = _notificationSignalMap.find(notification);
    if (iter == _notificationSignalMap.end())
    {
        _notificationSignalMap.insert(NotificationSignalMapEntry(notification, Signal()));
        iter = _notificationSignalMap.find(notification);
    }
    return iter->second;
}


Controller::Signal ControllerImpl::signalNotified(const RefPtr<RefObj>& object)
{
    Glib::RecMutex::Lock k(_mutex);
    void* key = const_cast<RefObj*>(object.ptr());
    RefObjSignalMap::iterator iter = _refObjSignalMap.find(key);
    if (iter == _refObjSignalMap.end())
    {
        _refObjSignalMap.insert(RefObjSignalMapEntry(key, Signal()));
        iter = _refObjSignalMap.find(key);
    }
    return iter->second;
}


void ControllerImpl::notify(const RefPtr<RefObj>& object, int notification)
{
    Trace trace("ControllerImpl::notify", "object=%zx notification=%d", object.ptr(), notification);
    if (_quitInProgress)
    {
        trace.put("cancelled.");
        return;
    }
    Glib::RecMutex::Lock k(_mutex);
    _notified.push_back(RefPtrNotificationPair(object, notification));
    if (_notified.size() == 1)
    {
        _dispatcher();
    }
}


void ControllerImpl::onNotify()
{
    Trace trace("ControllerImpl::onNotify");

    for (;;)
    {
        RefPtrNotificationPair entry;
        {
            Glib::RecMutex::Lock k(_mutex);
            if (!_notified.size())
            {
                break;
            }
            entry = _notified.front();
            _notified.pop_front();
        }
        {
            int key = entry.second;
            NotificationSignalMap::iterator iter = _notificationSignalMap.find(key);
            if (iter != _notificationSignalMap.end())
            {
                iter->second.emit(entry.first, entry.second);
            }
        }
        {
            void* key = entry.first.ptr();
            RefObjSignalMap::iterator iter = _refObjSignalMap.find(key);
            if (iter != _refObjSignalMap.end())
            {
                iter->second.emit(entry.first, entry.second);
                if (entry.second == XenObject::DESTROYED)
                {
                    _refObjSignalMap.erase(key);
                }
            }
        }
    }
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
