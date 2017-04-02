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

        typedef std::pair<ConsoleView*, ConsoleView::Message> VirtualMachineMessagePair;
        typedef std::list<VirtualMachineMessagePair> VirtualMachineMessageList;
        typedef std::map<void*, ConsoleViewSignal> ConsoleViewSignalMap;
        typedef std::pair<void*, ConsoleViewSignal> ConsoleViewSignalEntry;

        typedef std::pair<RefPtr<XenObject>, int> XenObjectNotificationPair;
        typedef std::list<XenObjectNotificationPair> XenObjectNotificationList;
        typedef std::map<int, XenObjectSignal> NotificationXenObjectSignalMap;
        typedef std::pair<int, XenObjectSignal> NotificationXenObjectSignalEntry;
        typedef std::map<void*, XenObjectSignal> XenObjectSignalMap;
        typedef std::pair<void*, XenObjectSignal> XenObjectSignalEntry;

        SignalManagerImpl();
        ~SignalManagerImpl();
        virtual void clear();
        virtual ConsoleViewSignal consoleViewSignal(const ConsoleView&);
        virtual void notify(const ConsoleView&, const ConsoleView::Message&);
        virtual XenObjectSignal xenObjectSignal(int);
        virtual XenObjectSignal xenObjectSignal(const XenObject&);
        virtual void notify(const RefPtr<XenObject>&, int);

    private:

        SignalManagerImpl(const SignalManagerImpl&);
        void operator =(const SignalManagerImpl&);
        inline int enqueue(const ConsoleView&, const ConsoleView::Message&);
        inline bool dequeue(ConsoleView*&, ConsoleView::Message&);
        inline int enqueue(const RefPtr<XenObject>&, int);
        inline bool dequeue(RefPtr<XenObject>&, int&);
        void onNotify();

        Glib::Dispatcher _dispatcher;

        // Priority 1
        Glib::Mutex _virtualMachineMessageMutex;
        VirtualMachineMessageList _virtualMachineMessageList;
        ConsoleViewSignalMap _consoleViewSignalMap;

        // Priority 2
        Glib::Mutex _xenObjectNotificationMutex;
        XenObjectNotificationList _xenObjectNotificationList;
        NotificationXenObjectSignalMap _notificationXenObjectSignalMap;
        XenObjectSignalMap _xenObjectSignalMap;
    };
}


#endif //!HNRT_SIGNALMANAGERIMPL_H
