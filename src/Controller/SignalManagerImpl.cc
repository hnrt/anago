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


inline int SignalManagerImpl::enqueue(const RefPtr<XenObject>& object, int notification)
{
    Glib::Mutex::Lock lock(_xenObjectNotificationMutex);
    _xenObjectNotificationList.push_back(XenObjectNotificationPair(object, notification));
    return static_cast<int>(_xenObjectNotificationList.size());
}


inline bool SignalManagerImpl::dequeue(RefPtr<XenObject>& object, int& notification)
{
    Glib::Mutex::Lock lock(_xenObjectNotificationMutex);
    if (_xenObjectNotificationList.size())
    {
        XenObjectNotificationPair entry;
        entry = _xenObjectNotificationList.front();
        _xenObjectNotificationList.pop_front();
        object = entry.first;
        notification = entry.second;
        return true;
    }
    else
    {
        return false;
    }
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
    {
        Glib::Mutex::Lock lock(_xenObjectNotificationMutex);
        _xenObjectNotificationList.clear();
    }
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
        Logger::instance().error("signal notification=%s requested in thread %s.", GetNotificationText(notification), ThreadManager::instance().getName());
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
        Logger::instance().error("signal object=%s@%zx requested in thread %s.", GetXenObjectTypeText(object), &object, ThreadManager::instance().getName());
        throw std::runtime_error("SignalManagerImpl::xenObjectSignal invoked from a background thread.");
    }
}


void SignalManagerImpl::notify(const RefPtr<XenObject>& object, int notification)
{
    TRACE("SignalManagerImpl::notify", "%s@%zx %s", GetXenObjectTypeText(*object), object.ptr(), GetNotificationText(notification));
    enqueue(object, notification);
    _dispatcher();
}


void SignalManagerImpl::onNotify()
{
    TRACE("SignalManagerImpl::onNotify");
    RefPtr<XenObject> object;
    int notification;
    if (dequeue(object, notification))
    {
        TRACEPUT("%s@%zx %s", GetXenObjectTypeText(*object), object.ptr(), GetNotificationText(notification));
        {
            NotificationXenObjectSignalMap::iterator iter = _notificationXenObjectSignalMap.find(notification);
            if (iter != _notificationXenObjectSignalMap.end())
            {
                iter->second.emit(object, notification);
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
