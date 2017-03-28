// Copyright (C) 2012-2017 Hideaki Narita


#include <stdexcept>
#include "Logger/Trace.h"
#include "Thread/ThreadManager.h"
#include "XenServer/XenObject.h"
#include "SignalManagerImpl.h"


using namespace hnrt;


SignalManagerImpl::SignalManagerImpl()
{
    TRACE("SignalManagerImpl::ctor");
    _dispatcher.connect(sigc::mem_fun(*this, &SignalManagerImpl::onNotify));
}


SignalManagerImpl::~SignalManagerImpl()
{
    TRACE("SignalManagerImpl::dtor");
}


void SignalManagerImpl::clear()
{
    TRACE("SignalManagerImpl::clear");
    if (ThreadManager::instance().isMain())
    {
        _notificationXenObjectSignalMap.clear();
        _xenObjectSignalMap.clear();
    }
    else
    {
        Logger::instance().error("signal clearance requested in thread %s.", ThreadManager::instance().getName());
        throw std::runtime_error("SignalManagerImpl::clear invoked from a background thread.");
    }
    Glib::Mutex::Lock lock(_mutex);
    _notified.clear();
}


SignalManager::XenObjectSignal SignalManagerImpl::xenObjectSignal(int notification)
{
    if (ThreadManager::instance().isMain())
    {
        NotificationXenObjectSignalMap::iterator iter = _notificationXenObjectSignalMap.find(notification);
        if (iter == _notificationXenObjectSignalMap.end())
        {
            _notificationXenObjectSignalMap.insert(NotificationXenObjectSignalEntry(notification, XenObjectSignal()));
            iter = _notificationXenObjectSignalMap.find(notification);
        }
        return iter->second;
    }
    else
    {
        Logger::instance().error("signal notification=%d requested in thread %s.", notification, ThreadManager::instance().getName());
        throw std::runtime_error("SignalManagerImpl::xenObjectSignal invoked from a background thread.");
    }
}


SignalManager::XenObjectSignal SignalManagerImpl::xenObjectSignal(const XenObject& object)
{
    if (ThreadManager::instance().isMain())
    {
        void* key = const_cast<XenObject*>(&object);
        XenObjectSignalMap::iterator iter = _xenObjectSignalMap.find(key);
        if (iter == _xenObjectSignalMap.end())
        {
            _xenObjectSignalMap.insert(XenObjectSignalEntry(key, XenObjectSignal()));
            iter = _xenObjectSignalMap.find(key);
        }
        return iter->second;
    }
    else
    {
        Logger::instance().error("signal object=%zx requested in thread %s.", &object, ThreadManager::instance().getName());
        throw std::runtime_error("SignalManagerImpl::xenObjectSignal invoked from a background thread.");
    }
}


inline int SignalManagerImpl::enqueue(const RefPtr<XenObject>& object, int notification)
{
    Glib::Mutex::Lock lock(_mutex);
    _notified.push_back(XenObjectNotificationPair(object, notification));
    return static_cast<int>(_notified.size());
}


inline bool SignalManagerImpl::dequeue(RefPtr<XenObject>& object, int& notification)
{
    Glib::Mutex::Lock lock(_mutex);
    if (_notified.size())
    {
        XenObjectNotificationPair entry;
        entry = _notified.front();
        _notified.pop_front();
        object = entry.first;
        notification = entry.second;
        return true;
    }
    else
    {
        return false;
    }
}


void SignalManagerImpl::notify(const RefPtr<XenObject>& object, int notification)
{
    TRACE("SignalManagerImpl::notify", "object=%zx notification=%d", object.ptr(), notification);
    int size = enqueue(object, notification);
    if (ThreadManager::instance().isMain())
    {
        onNotify();
    }
    else if (size == 1)
    {
        _dispatcher();
    }
}


void SignalManagerImpl::onNotify()
{
    TRACE("SignalManagerImpl::onNotify");
    RefPtr<XenObject> object;
    int notification;
    while (dequeue(object, notification))
    {
        TRACEPUT("object=%zx notification=%d", object.ptr(), notification);
        {
            NotificationXenObjectSignalMap::iterator iter = _notificationXenObjectSignalMap.find(notification);
            if (iter != _notificationXenObjectSignalMap.end())
            {
                iter->second.emit(object, notification);
                continue;
            }
        }
        {
            XenObjectSignalMap::iterator iter = _xenObjectSignalMap.find(object.ptr());
            if (iter != _xenObjectSignalMap.end())
            {
                iter->second.emit(object, notification);
                if (notification == XenObject::DESTROYED)
                {
                    _xenObjectSignalMap.erase(iter);
                }
            }
        }
    }
}
