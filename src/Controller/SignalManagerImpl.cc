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
    _dispatcher1.connect(sigc::mem_fun(*this, &SignalManagerImpl::onNotify1));
    _dispatcher2.connect(sigc::mem_fun(*this, &SignalManagerImpl::onNotify2));
}


SignalManagerImpl::~SignalManagerImpl()
{
    TRACE("SignalManagerImpl::dtor");
}


inline int SignalManagerImpl::enqueue(const ConsoleView& cv, const ConsoleView::Message& message)
{
    Glib::Mutex::Lock lock(_virtualMachineMessageMutex);
    _virtualMachineMessageList.push_back(VirtualMachineMessagePair(const_cast<ConsoleView*>(&cv), message));
    return static_cast<int>(_virtualMachineMessageList.size());
}


inline bool SignalManagerImpl::dequeue(ConsoleView*& cv, ConsoleView::Message& message)
{
    Glib::Mutex::Lock lock(_virtualMachineMessageMutex);
    if (_virtualMachineMessageList.size())
    {
        VirtualMachineMessagePair entry = _virtualMachineMessageList.front();
        _virtualMachineMessageList.pop_front();
        cv = entry.first;
        message = entry.second;
        return true;
    }
    else
    {
        return false;
    }
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
        _consoleViewSignalMap.clear();
        _notificationXenObjectSignalMap.clear();
        _xenObjectSignalMap.clear();
    }
    else
    {
        Logger::instance().error("signal clearance requested in thread %s.", ThreadManager::instance().getName());
        throw std::runtime_error("SignalManagerImpl::clear invoked from a background thread.");
    }
    {
        Glib::Mutex::Lock lock(_virtualMachineMessageMutex);
        _virtualMachineMessageList.clear();
    }
    {
        Glib::Mutex::Lock lock(_xenObjectNotificationMutex);
        _xenObjectNotificationList.clear();
    }
}


SignalManagerImpl::ConsoleViewSignal SignalManagerImpl::consoleViewSignal(const ConsoleView& cv)
{
    if (ThreadManager::instance().isMain())
    {
        void* key = const_cast<ConsoleView*>(&cv);
        ConsoleViewSignalMap::iterator iter = _consoleViewSignalMap.find(key);
        if (iter == _consoleViewSignalMap.end())
        {
            _consoleViewSignalMap.insert(ConsoleViewSignalEntry(key, ConsoleViewSignal()));
            iter = _consoleViewSignalMap.find(key);
        }
        return iter->second;
    }
    else
    {
        Logger::instance().error("signal cv=%zx requested in thread %s.", &cv, ThreadManager::instance().getName());
        throw std::runtime_error("SignalManagerImpl::consoleViewSignal invoked from a background thread.");
    }
}


void SignalManagerImpl::notify(const ConsoleView& cv, const ConsoleView::Message& message)
{
    TRACE("SignalManagerImpl::notify", "cv=%zx message.type=%d", &cv, message.type);
    enqueue(cv, message);
    _dispatcher1();
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


void SignalManagerImpl::notify(const RefPtr<XenObject>& object, int notification)
{
    TRACE("SignalManagerImpl::notify", "object=%zx notification=%d", object.ptr(), notification);
    enqueue(object, notification);
    _dispatcher2();
}


void SignalManagerImpl::onNotify1()
{
    TRACE("SignalManagerImpl::onNotify1");
    ConsoleView* cv = NULL;
    ConsoleView::Message message;
    if (dequeue(cv, message))
    {
        TRACEPUT("cv=%zx message.type=%d", cv, message.type);
        ConsoleViewSignalMap::iterator iter = _consoleViewSignalMap.find(cv);
        if (iter != _consoleViewSignalMap.end())
        {
            iter->second.emit(message);
        }
    }
}


void SignalManagerImpl::onNotify2()
{
    TRACE("SignalManagerImpl::onNotify2");
    RefPtr<XenObject> object;
    int notification;
    if (dequeue(object, notification))
    {
        TRACEPUT("object=%zx notification=%d", object.ptr(), notification);
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
