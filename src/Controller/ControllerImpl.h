// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONTROLLERIMPL_H
#define HNRT_CONTROLLERIMPL_H


#include <glibmm.h>
#include <list>
#include <map>
#include "Controller.h"


namespace hnrt
{
    class Host;
    class PerformanceMonitor;

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
        virtual Signal signalNotified(int);
        virtual Signal signalNotified(const RefPtr<RefObj>&);
        virtual void notify(const RefPtr<RefObj>&, int);
        virtual void addHost();
        virtual void editHost();
        virtual void removeHost();
        virtual void connect();
        virtual void disconnect();
        virtual void changeHostName();
        virtual void wakeHost();
        virtual void shutdownHosts();
        virtual void restartHosts();
        virtual void startVm();
        virtual void shutdownVm();
        virtual void rebootVm();
        virtual void suspendVm();
        virtual void resumeVm();
        virtual void changeCd();
        virtual void sendCtrlAltDelete();
        virtual void addVm();
        virtual void copyVm();
        virtual void deleteVm();
        virtual void snapshotVm();
        virtual void exportVm();
        virtual void importVm();
        virtual void verifyVm();
        virtual void hardShutdownVm();
        virtual void hardRebootVm();
        virtual void changeVmName();
        virtual void changeCpu();
        virtual void changeMemory();
        virtual void changeShadowMemory();
        virtual void changeVga();
        virtual void attachHdd();
        virtual void attachCd();
        virtual void attachNic();
        virtual void addHdd();
        virtual void addCifs();
        virtual void deleteCifs();
        virtual void changeSrName();
        virtual void setDefaultSr();
        virtual void openVmStatusWindow();
        virtual void showAbout();

    private:

        ControllerImpl(const ControllerImpl&);
        void operator =(const ControllerImpl&);
        bool quit2();
        void onNotify();
        void onConnectFailed(RefPtr<RefObj>, int);
        void onXenObjectError(RefPtr<RefObj>, int);
        void onXenTaskUpdated(RefPtr<RefObj>, int);
        void connectInBackground(RefPtr<Host>);
        void performanceMonitorInBackground(RefPtr<PerformanceMonitor>);
        void disconnectInBackground(RefPtr<Host>);

        bool _quitInProgress;
        Glib::RecMutex _mutex;
        std::list<RefPtrNotificationPair> _notified;
        Glib::Dispatcher _dispatcher;
        NotificationSignalMap _notificationSignalMap;
        RefObjSignalMap _refObjSignalMap;
    };
}


#endif //!HNRT_CONTROLLERIMPL_H
