// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINE_H
#define HNRT_VIRTUALMACHINE_H


#include <glibmm.h>
#include <list>
#include "XenObject.h"
#include "XenPtr.h"


namespace hnrt
{
    class VirtualDiskImage;
    class VirtualBlockDevice;

    class VirtualMachine
        : public XenObject
    {
    public:

        static RefPtr<VirtualMachine> create(Session&, xen_vm, const XenPtr<xen_vm_record>&);

        virtual ~VirtualMachine();
        virtual int setBusy(bool = true);
        XenPtr<xen_vm_record> getRecord();
        void setRecord(const XenPtr<xen_vm_record>&);
        XenPtr<xen_vm_metrics_record> getMetricsRecord();
        void setRecord(const XenPtr<xen_vm_metrics_record>&);
        XenPtr<xen_vm_guest_metrics_record> getGuestMetricsRecord();
        void setRecord(const XenPtr<xen_vm_guest_metrics_record>&);
        int getVbds(std::list<RefPtr<VirtualBlockDevice> >&);
        RefPtr<VirtualBlockDevice> getVbd(const RefPtr<VirtualDiskImage>&);
        bool setName(const char* label, const char* description);
        bool setVcpu(int64_t vcpusMax, int64_t vcpusAtStartup, int coresPerSocket);
        int getCoresPerSocket();
        bool setCoresPerSocket(int);
        bool setMemory(int64_t staticMin, int64_t staticMax, int64_t dynamicMin, int64_t dynamicMax);
        bool setShadowMemory(double);
        bool start();
        bool shutdown(bool = false);
        bool reboot(bool = false);
        bool suspend();
        bool resume();
        bool changeCd(xen_vbd, xen_vdi);
        bool isStdVga();
        bool setStdVga(bool stdVga);
        Glib::ustring getVga();
        bool setVga(const char*);
        int getVideoRam();
        bool setVideoRam(int);
        Glib::ustring getPrimarySr();
        bool clone(const char*);
        bool copy(const char*, xen_sr);
        bool destroy();
        Glib::ustring getConsoleLocation();

    protected:

        VirtualMachine(Session&, xen_vm, const XenPtr<xen_vm_record>&);
        VirtualMachine(const VirtualMachine&);
        void operator =(const VirtualMachine&);

        XenPtr<xen_vm_record> _record;
        XenPtr<xen_vm_metrics_record> _metricsRecord;
        XenPtr<xen_vm_guest_metrics_record> _guestMetricsRecord;
    };
}


#endif //!HNRT_VIRTUALMACHINE_H
