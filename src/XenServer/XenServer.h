// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_XENSERVER_H
#define HNRT_XENSERVER_H


#include <glibmm/ustring.h>
#include <vector>
#include "XenServer/Api.h"


namespace hnrt
{
    class StringBuffer;
    struct CifsSpec;
    struct HardDiskDriveSpec;
    struct VirtualMachineSpec;

    struct XenServer
    {
        static StringBuffer& getError(xen_session* session, StringBuffer& buffer, const char* separator);
        static std::vector<Glib::ustring>& getError(xen_session* session, std::vector<Glib::ustring>& array);
        static const char* getErrorMessage(xen_session* session);
        static void setError(xen_session* session, const char* error, ...);
        static void setError(xen_session* session, const std::vector<Glib::ustring>& array);
        static bool getErrorFromTask(xen_session* session, xen_task task, StringBuffer& buffer, const char* separator);
        static const char* getPowerStateText(int);
        static Glib::ustring getOs(const xen_vm_guest_metrics_record* record);
        static Glib::ustring getIp(const xen_vm_guest_metrics_record* record, int device);
        static Glib::ustring getIpv6(const xen_vm_guest_metrics_record* record, int device);
        static const char* getParent(const xen_vdi_record* record);
        static Glib::ustring getDefaultSr(xen_session* session);
        static int getDefaultSr(xen_session* session, std::vector<Glib::ustring>& array);
        static bool isDefaultSr(xen_session* session, xen_sr sr);
        static bool setDefaultSr(xen_session* session, xen_sr sr);
        static int64_t getDiskSizeHint(const xen_vm_record *record);
        static const char* find(const xen_string_string_map*, const char* key);
        static int match(const xen_string_string_map*, const char* key);
        static int match(const xen_string_string_map*, const char* key, const char* value1);
        static int match(const xen_string_string_map*, const char* key, const char* value1, const char* value2);
        static const char* getText(enum xen_vbd_type);
        static bool createVirtualMachine(xen_session*, const VirtualMachineSpec&, xen_vm*);
        static bool createHdd(xen_session*, xen_vm, const char*, const HardDiskDriveSpec&);
        static bool attachHdd(xen_session*, xen_vm, const char*, xen_vdi);
        static bool createCd(xen_session*, xen_vm, const char*, xen_vdi);
        static bool attachCd(xen_session*, xen_vm, const char*);
        static bool createNic(xen_session*, xen_vm, const char*, xen_network);
        static bool createVdi(xen_session*, const HardDiskDriveSpec&, xen_vdi*);
        static bool setVmHintToVdi(xen_session*, xen_vdi, xen_vm);
        static bool createVbd(xen_session*, xen_vm, const char*, xen_vdi, enum xen_vbd_type, enum xen_vbd_mode, bool, xen_vbd*);
        static bool createVif(xen_session*, xen_vm, const char*, xen_network, xen_vif*);
        static bool createSnapshot(xen_session*, xen_vm);
        static bool addCifs(xen_session*, xen_host, const CifsSpec&, xen_sr*);

#define XenPtrFree(type) static void free(type* p) { if (p) type##_free(p); }

        XenPtrFree(xen_console_record)
        XenPtrFree(xen_console_set)
        XenPtrFree(xen_event_record_set)
        XenPtrFree(xen_host_metrics_record)
        XenPtrFree(xen_host_patch_record)
        XenPtrFree(xen_host_patch_set)
        XenPtrFree(xen_host_record)
        XenPtrFree(xen_network_record)
        XenPtrFree(xen_network_set)
        XenPtrFree(xen_pif_record)
        XenPtrFree(xen_pif_set)
        XenPtrFree(xen_pbd_record)
        XenPtrFree(xen_pbd_set)
        XenPtrFree(xen_pool_set)
        XenPtrFree(xen_pool_patch_record)
        XenPtrFree(xen_pool_patch_set)
        XenPtrFree(xen_pool_update_record)
        XenPtrFree(xen_pool_update_set)
        XenPtrFree(xen_sr_record)
        XenPtrFree(xen_sr_set)
        XenPtrFree(xen_string_set)
        XenPtrFree(xen_task_record)
        XenPtrFree(xen_vbd_record)
        XenPtrFree(xen_vbd_set)
        XenPtrFree(xen_vdi_record)
        XenPtrFree(xen_vdi_set)
        XenPtrFree(xen_vif_record)
        XenPtrFree(xen_vif_set)
        XenPtrFree(xen_vm_guest_metrics_record)
        XenPtrFree(xen_vm_metrics_record)
        XenPtrFree(xen_vm_record)
        XenPtrFree(xen_vm_set)
    };
}


#endif //!HNRT_XENSERVER_H
