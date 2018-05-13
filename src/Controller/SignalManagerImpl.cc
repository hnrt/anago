// Copyright (C) 2012-2018 Hideaki Narita


#include <stdexcept>
#include "Logger/Trace.h"
#include "Thread/ThreadManager.h"
#include "XenServer/XenObject.h"
#include "SignalManagerImpl.h"


using namespace hnrt;


SignalManagerImpl::SignalManagerImpl()
{
    TRACEFUN(NULL, "SignalManagerImpl::ctor");
    _dispatcher.connect(sigc::mem_fun(*this, &SignalManagerImpl::onNotify));
}


SignalManagerImpl::~SignalManagerImpl()
{
    TRACEFUN(NULL, "SignalManagerImpl::dtor");
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
    TRACEFUN(NULL, "SignalManagerImpl::clear");
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
    TRACE("SignalManagerImpl::xenObjectSignal(%s)", GetNotificationText(notification));
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
    TRACE("SignalManagerImpl::xenObjectSignal(%s)", XenObjectText(object).ptr);
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
    TRACEFUN(NULL, "SignalManagerImpl::notify(%s@%zx,%s)", GetXenObjectTypeText(*object), object.ptr(), GetNotificationText(notification));
    enqueue(object, notification);
    _dispatcher();
}


void SignalManagerImpl::onNotify()
{
    RefPtr<XenObject> object;
    int notification;
    if (dequeue(object, notification))
    {
        int hit = 0;
        XenObjectSignal sig1, sig2;
        {
            NotificationXenObjectSignalMap::iterator iter = _notificationXenObjectSignalMap.find(notification);
            if (iter != _notificationXenObjectSignalMap.end())
            {
                sig1 = iter->second;
                hit++;
            }
        }
        {
            XenObjectSignalMap::iterator iter = _xenObjectSignalMap.find(object.ptr());
            if (iter != _xenObjectSignalMap.end())
            {
                sig2 = iter->second;
                hit++;
                if (notification == XenObject::DESTROYED)
                {
                    _xenObjectSignalMap.erase(iter);
                }
            }
        }
        if (!sig1.empty())
        {
            TRACEFUN(NULL, "SignalManagerImpl::onNotify(%s,*%s*)%s", XenObjectText(*object).ptr, GetNotificationText(notification), hit == 2 ? "[1/2]" : "");
            sig1.emit(object, notification);
        }
        if (!sig2.empty())
        {
            TRACEFUN(NULL, "SignalManagerImpl::onNotify(*%s*,%s)%s", XenObjectText(*object).ptr, GetNotificationText(notification), hit == 2 ? "[2/2]" : "");
            sig2.emit(object, notification);
        }
        if (!hit)
        {
            TRACE("SignalManagerImpl::onNotify(%s,%s): No callback.", XenObjectText(*object).ptr, GetNotificationText(notification));
        }
    }
}
