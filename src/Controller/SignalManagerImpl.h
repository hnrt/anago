// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_SIGNALMANAGERIMPL_H
#define HNRT_SIGNALMANAGERIMPL_H


#include <glibmm.h>
#include <list>
#include <map>
#include "SignalManager.h"


namespace hnrt
{
    class SignalManagerImpl
        : public SignalManager
    {
    public:

        typedef std::map<int, XenObjectSignal> NotificationXenObjectSignalMap;
        typedef std::pair<int, XenObjectSignal> NotificationXenObjectSignalEntry;
        typedef std::map<void*, XenObjectSignal> XenObjectSignalMap;
        typedef std::pair<void*, XenObjectSignal> XenObjectSignalEntry;
        typedef std::pair<RefPtr<XenObject>, int> XenObjectNotificationPair;
        typedef std::list<XenObjectNotificationPair> XenObjectNotificationList;

        SignalManagerImpl();
        ~SignalManagerImpl();
        virtual void clear();
        virtual XenObjectSignal xenObjectSignal(int);
        virtual XenObjectSignal xenObjectSignal(const XenObject&);
        virtual void notify(const RefPtr<XenObject>&, int);

    private:

        SignalManagerImpl(const SignalManagerImpl&);
        void operator =(const SignalManagerImpl&);
        inline int enqueue(const RefPtr<XenObject>& object, int notification);
        inline bool dequeue(RefPtr<XenObject>& object, int& notification);
        void onNotify();


        Glib::Dispatcher _dispatcher;
        Glib::Mutex _mutex;
        NotificationXenObjectSignalMap _notificationXenObjectSignalMap;
        XenObjectSignalMap _xenObjectSignalMap;
        XenObjectNotificationList _notified;
    };
}


#endif //!HNRT_SIGNALMANAGERIMPL_H
