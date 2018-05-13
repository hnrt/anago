// Copyright (C) 2012-2018 Hideaki Narita


#include <glibmm.h>
#include "Base/Atomic.h"
#include "Controller/SignalManager.h"
#include "Constants.h"
#include "Session.h"
#include "XenObject.h"


using namespace hnrt;


XenObject::XenObject(Type type, Session& session, void* handle, const char* uuid, const char* name)
    : _type(type)
    , _session(session)
    , _refid(handle ? reinterpret_cast<char*>(handle) : NULLREFSTRING)
    , _uuid(uuid ? uuid : "")
    , _name(name ? name : "")
    , _busyCount(0)
{
    if (_type != SESSION)
    {
        _session.incRef();
    }
    _handle = const_cast<void*>(reinterpret_cast<const void*>(_refid.c_str()));
}


XenObject::~XenObject()
{
    if (_type != SESSION)
    {
        _session.decRef();
    }
}


Glib::ustring XenObject::getName() const
{
    Glib::Mutex::Lock lock(const_cast<XenObject*>(this)->_mutex);
    return _name;
}


void XenObject::setName(const char* value)
{
    if (value)
    {
        Glib::Mutex::Lock lock(_mutex);
        if (_name == value)
        {
            return;
        }
        _name = value;
    }
    else
    {
        Glib::Mutex::Lock lock(_mutex);
        if (_name.empty())
        {
            return;
        }
        _name.clear();
    }
    emit(NAME_UPDATED);
}


Glib::ustring XenObject::getDisplayStatus() const
{
    Glib::Mutex::Lock lock(const_cast<XenObject*>(this)->_mutex);
    return _displayStatus;
}


void XenObject::setDisplayStatus(const char* value)
{
    if (value)
    {
        Glib::Mutex::Lock lock(_mutex);
        if (_displayStatus == value)
        {
            return;
        }
        _displayStatus = value;
    }
    else
    {
        Glib::Mutex::Lock lock(_mutex);
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
    SignalManager::instance().notify(RefPtr<XenObject>(this, 1), notification);
}
