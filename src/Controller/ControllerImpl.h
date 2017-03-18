// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONTROLLERIMPL_H
#define HNRT_CONTROLLERIMPL_H


#include <glibmm.h>
#include <list>
#include <map>
#include "Controller.h"


namespace hnrt
{
    class ControllerImpl
        : public sigc::trackable
        , public Controller
    {
    public:

        typedef std::map<int, Signal> NotificationSignalMap;
        typedef std::pair<int, Signal> NotificationSignalMapEntry;
        typedef std::map<void*, Signal> RefObjSignalMap;
        typedef std::pair<void*, Signal> RefObjSignalMapEntry;
        typedef std::pair<RefPtr<RefObj>, int> RefPtrNotificationPair;

        ControllerImpl();
        ~ControllerImpl();
        virtual void clear();
        virtual void parseCommandLine(int argc, char *argv[]);
        virtual void quit();
        virtual void incBackgroundCount();
        virtual void decBackgroundCount();
        virtual Signal signalNotified(int);
        virtual Signal signalNotified(const RefPtr<RefObj>&);
        virtual void notify(const RefPtr<RefObj>&, int);
        virtual void addHost();
        virtual void editHost();
        virtual void removeHost();
        virtual void connect();
        virtual void disconnect();
        virtual void showAbout();

    private:

        ControllerImpl(const ControllerImpl&);
        void operator =(const ControllerImpl&);
        bool quit2();
        void onNotify();

        volatile int _backgroundCount;
        bool _quitInProgress;
        Glib::RecMutex _mutex;
        std::list<RefPtrNotificationPair> _notified;
        Glib::Dispatcher _dispatcher;
        NotificationSignalMap _notificationSignalMap;
        RefObjSignalMap _refObjSignalMap;
    };
}


#endif //!HNRT_CONTROLLERIMPL_H
