// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONTROLLERIMPL_H
#define HNRT_CONTROLLERIMPL_H


#include <sigc++/sigc++.h>
#include "Base/RefPtr.h"
#include "Controller.h"


namespace hnrt
{
    class Host;
    class PerformanceMonitor;
    class XenObject;

    class ControllerImpl
        : public sigc::trackable
        , public Controller
    {
    public:

        ControllerImpl();
        ~ControllerImpl();
        virtual void parseCommandLine(int argc, char *argv[]);
        virtual void quit();
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
        void onConnectFailed(RefPtr<XenObject>, int);
        void onXenObjectError(RefPtr<XenObject>, int);
        void onXenTaskUpdated(RefPtr<XenObject>, int);
        void connectInBackground(RefPtr<Host>);
        void performanceMonitorInBackground(RefPtr<PerformanceMonitor>);
        void disconnectInBackground(RefPtr<Host>);

        bool _quitInProgress;
    };
}


#endif //!HNRT_CONTROLLERIMPL_H
