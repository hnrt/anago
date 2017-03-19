// Copyright (C) 2012-2017 Hideaki Narita


#include <glibmm.h>
#include "Base/Atomic.h"
#include "Controller/Controller.h"
#include "Logger/Trace.h"
#include "Session.h"
#include "XenObject.h"
#include "Constants.h"


using namespace hnrt;


XenObject::XenObject(Type type, Session& session, const char* refid, const char* uuid, const char* name)
    : _type(type)
    , _session(session)
    , _refid(refid ? refid : NULLREFSTRING)
    , _uuid(uuid ? uuid : "")
    , _name(name ? name : "")
    , _busyCount(0)
{
    if (_type != SESSION)
    {
        _session.reference();
    }
}


XenObject::~XenObject()
{
    if (_type != SESSION)
    {
        _session.unreference();
    }
}


const Session& XenObject::getSession() const
{
    return _session;
}


Session& XenObject::getSession()
{
    return _session;
}


Glib::ustring XenObject::getName()
{
    Glib::Mutex::Lock k(_mutex);
    return _name;
}


void XenObject::setName(const char* value)
{
    if (value)
    {
        Glib::Mutex::Lock k(_mutex);
        if (_name == value)
        {
            return;
        }
        _name = value;
    }
    else
    {
        Glib::Mutex::Lock k(_mutex);
        if (_name.empty())
        {
            return;
        }
        _name.clear();
    }
    emit(NAME_UPDATED);
}


Glib::ustring XenObject::getDisplayStatus()
{
    Glib::Mutex::Lock k(_mutex);
    return _displayStatus;
}


void XenObject::setDisplayStatus(const char* value)
{
    if (value)
    {
        Glib::Mutex::Lock k(_mutex);
        if (_displayStatus == value)
        {
            return;
        }
        _displayStatus = value;
    }
    else
    {
        Glib::Mutex::Lock k(_mutex);
        if (_displayStatus.empty())
        {
            return;
        }
        _displayStatus.clear();
    }
    emit(STATUS_UPDATED);
}


int XenObject::setBusy(bool value)
{
    int count;
    if (value)
    {
        count = InterlockedIncrement(&_busyCount);
        if (count == 1)
        {
            emit(BUSY_SET);
        }
    }
    else
    {
        count = InterlockedDecrement(&_busyCount);
        if (count == 0)
        {
            emit(BUSY_RESET);
        }
    }
    return count;
}


void XenObject::emit(Notification notification)
{
    Controller::instance().notify(RefPtr<RefObj>(this, 1), notification);
}
