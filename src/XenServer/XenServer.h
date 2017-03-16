// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_XENSERVER_H
#define HNRT_XENSERVER_H


#include <glibmm.h>
#include <vector>
#include "Util/StringBuffer.h"
#include "XenServer/Api.h"


namespace hnrt
{
    struct XenServer
    {
        static StringBuffer& getError(xen_session* session, StringBuffer& buffer, const char* separator);
        static std::vector<Glib::ustring>& getError(xen_session* session, std::vector<Glib::ustring>& array);
        static const char* getErrorMessage(xen_session* session);
        static void setError(xen_session* session, const char* error, ...);
        static void setError(xen_session* session, const std::vector<Glib::ustring>& array);
        static bool getErrorFromTask(xen_session* session, xen_task task, StringBuffer& buffer, const char* separator);
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
