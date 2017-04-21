// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONTROLLER_H
#define HNRT_CONTROLLER_H


namespace hnrt
{
    class VirtualBlockDevice;

    class Controller
    {
    public:

        static void init();
        static void fini();
        static Controller& instance();

        virtual void parseCommandLine(int argc, char *argv[]) = 0;
        virtual void quit() = 0;
        virtual void addHost() = 0;
        virtual void editHost() = 0;
        virtual void removeHost() = 0;
        virtual void connect() = 0;
        virtual void disconnect() = 0;
        virtual void changeHostName() = 0;
        virtual void wakeHost() = 0;
        virtual void shutdownHosts() = 0;
        virtual void restartHosts() = 0;
        virtual void startVm() = 0;
        virtual void shutdownVm() = 0;
        virtual void rebootVm() = 0;
        virtual void suspendVm() = 0;
        virtual void resumeVm() = 0;
        virtual void changeCd() = 0;
        virtual void changeCd2(const VirtualBlockDevice&) = 0;
        virtual void sendCtrlAltDelete() = 0;
        virtual void addVm() = 0;
        virtual void copyVm() = 0;
        virtual void deleteVm() = 0;
        virtual void exportVm() = 0;
        virtual void importVm() = 0;
        virtual void verifyVm() = 0;
        virtual void hardShutdownVm() = 0;
        virtual void hardRebootVm() = 0;
        virtual void changeVmName() = 0;
        virtual void changeCpu() = 0;
        virtual void changeMemory() = 0;
        virtual void changeShadowMemory() = 0;
        virtual void changeVga() = 0;
        virtual void attachHdd() = 0;
        virtual void attachCd() = 0;
        virtual void attachNic() = 0;
        virtual void addHdd() = 0;
        virtual void addCifs() = 0;
        virtual void deleteCifs() = 0;
        virtual void changeSrName() = 0;
        virtual void setDefaultSr() = 0;
        virtual void openVmStatusWindow() = 0;
        virtual void changeSnapshotName() = 0;
        virtual void snapshotVm() = 0;
        virtual void revertVm() = 0;
        virtual void deleteSnapshot() = 0;
        virtual void showAbout() = 0;
    };
}


#endif //!HNRT_CONTROLLER_H
