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
    Trace trace(__PRETTY_FUNCTION__);

    // trace.put("type=%s", );
    // DBG("type=%s", XenObjectTypeMap::toString(_type));

    if (_type != SESSION)
    {
        _session.reference();
    }
}


XenObject::~XenObject()
{
    Trace trace(__PRETTY_FUNCTION__);

    //DBG("type=%s", XenObjectTypeMap::toString(_type));

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


void XenObject::lock()
{
    _mutex.lock();
}


void XenObject::unlock()
{
    _mutex.unlock();
}


Glib::ustring XenObject::getName()
{
    Glib::RecMutex::Lock k(_mutex);
    return _name;
}


void XenObject::setName(const char* value)
{
    {
        Glib::RecMutex::Lock k(_mutex);
        if (_name == value)
        {
            return;
        }
        _name = value;
    }
    emit(Controller::XO_NAME);
}


Glib::ustring XenObject::getDisplayStatus()
{
    Glib::RecMutex::Lock k(_mutex);
    return _displayStatus;
}


void XenObject::setDisplayStatus(const char* value)
{
    {
        Glib::RecMutex::Lock k(_mutex);
        if (_displayStatus == value)
        {
            return;
        }
        _displayStatus = value;
    }
    emit(Controller::XO_STATUS);
}


void XenObject::setBusy(bool value)
{
    if (value)
    {
        if (InterlockedIncrement(&_busyCount) != 1)
        {
            return;
        }
    }
    else if (InterlockedDecrement(&_busyCount) != 0)
    {
        return;
    }
    emit(Controller::XO_BUSY);
}


void XenObject::emit(int notification)
{
    Controller::instance().notify(RefPtr<RefObj>(this, 1), static_cast<Controller::Notification>(notification));
}
