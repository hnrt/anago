// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONTROLLERIMPL_H
#define HNRT_CONTROLLERIMPL_H


#include <glibmm.h>
#include <sigc++/sigc++.h>
#include "Base/RefPtr.h"
#include "XenServer/CifsSpec.h"
#include "XenServer/VirtualMachineSpec.h"
#include "XenServer/HardDiskDriveSpec.h"
#include "Controller.h"


namespace hnrt
{
    class Host;
    class PerformanceMonitor;
    class StorageRepository;
    class ThreadManager;
    class VirtualBlockDevice;
    class VirtualMachine;
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
        virtual void changeCd2(const VirtualBlockDevice&);
        virtual void sendCtrlAltDelete();
        virtual void addVm();
        virtual void copyVm();
        virtual void deleteVm();
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
        virtual void detachHdd(VirtualBlockDevice&);
        virtual void attachCd();
        virtual void attachNic();
        virtual void detachNic(VirtualInterface&);
        virtual void addCifs();
        virtual void deleteCifs();
        virtual void changeSrName();
        virtual void setDefaultSr();
        virtual void addHdd();
        virtual void addHddTo(StorageRepository&);
        virtual void openVmStatusWindow();
        virtual void changeSnapshotName();
        virtual void snapshotVm();
        virtual void revertVm();
        virtual void deleteSnapshot();
        virtual void changeVdiName(VirtualDiskImage&);
        virtual void resizeVdi(VirtualDiskImage&);
        virtual void removeVdi(VirtualDiskImage&);
        virtual void showAbout();

    private:

        ControllerImpl(const ControllerImpl&);
        void operator =(const ControllerImpl&);
        bool quit2();
        void backgroundWorker();
        void schedule(const sigc::slot<void>&);
        void connectAtStartup();
        void onConnectFailed(RefPtr<XenObject>, int);
        void onXenObjectError(RefPtr<XenObject>, int);
        void onXenTaskUpdated(RefPtr<XenObject>, int);
        void connectInBackground(RefPtr<Host>);
        void performanceMonitorInBackground(RefPtr<PerformanceMonitor>);
        void disconnect(const RefPtr<Host>&);
        void disconnectInBackground(RefPtr<Host>);
        void shutdownHostInBackground(RefPtr<Host>);
        void restartHostInBackground(RefPtr<Host>);
        void controlVm(bool (VirtualMachine::*memfunc)());
        void controlVm(bool (VirtualMachine::*memfunc)(bool), bool);
        void addVmInBackground(RefPtr<Host>, VirtualMachineSpec);
        void cloneVmInBackground(RefPtr<VirtualMachine>, Glib::ustring);
        void copyVmInBackground(RefPtr<VirtualMachine>, Glib::ustring, Glib::ustring);
        void deleteVmInBackground(RefPtr<VirtualMachine>, std::list<Glib::ustring>);
        void exportVmInBackground(RefPtr<VirtualMachine>, Glib::ustring, bool);
        void importVmInBackground(RefPtr<Host>, Glib::ustring);
        void verifyVmInBackground(Glib::ustring);
        void attachHddInBackground(RefPtr<VirtualMachine>, Glib::ustring, Glib::ustring);
        void detachHddInBackground(RefPtr<VirtualBlockDevice>);
        void attachCdInBackground(RefPtr<VirtualMachine>, Glib::ustring);
        void addHddInBackground(RefPtr<StorageRepository>, HardDiskDriveSpec);
        void attachNicInBackground(RefPtr<VirtualMachine>, Glib::ustring, Glib::ustring);
        void detachNicInBackground(RefPtr<VirtualInterface>);
        void snapshotVmInBackground(RefPtr<VirtualMachine>);
        void revertVmInBackground(RefPtr<VirtualMachine>);
        void addCifsInBackground(RefPtr<Host>, CifsSpec);
        void deleteCifsInBackground(RefPtr<StorageRepository>);

        ThreadManager& _tm;
        bool _quitInProgress;
        Glib::Mutex _mutexBackground;
        Glib::Cond _condBackground;
        std::list<sigc::slot<void> > _backgroundQueue;
    };
}


#endif //!HNRT_CONTROLLERIMPL_H
