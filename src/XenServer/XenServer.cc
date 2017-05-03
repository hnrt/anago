// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Util/Util.h"
#include "Constants.h"
#include "HardDiskDriveSpec.h"
#include "Macros.h"
#include "VirtualMachineSpec.h"
#include "XenPtr.h"
#include "XenRef.h"
#include "XenServer.h"


using namespace hnrt;


StringBuffer& XenServer::getError(xen_session* session, StringBuffer& buffer, const char* separator)
{
    if (!session || session->ok || !session->error_description_count || !session->error_description)
    {
        return buffer;
    }
    if (buffer.len())
    {
        buffer += separator;
    }
    const char* text = getErrorMessage(session);
    if (text)
    {
        buffer += text;
    }
    else
    {
        buffer += session->error_description[0];
        for (int i = 1; i < session->error_description_count; i++)
        {
            buffer += separator;
            buffer += session->error_description[i];
        }
    }
    return buffer;
}


std::vector<Glib::ustring>& XenServer::getError(xen_session* session, std::vector<Glib::ustring>& array)
{
    if (!session || session->ok || !session->error_description_count || !session->error_description)
    {
        return array;
    }
    const char* text = getErrorMessage(session);
    if (text)
    {
        array.push_back(Glib::ustring(text));
    }
    else
    {
        for (int i = 0; i < session->error_description_count; i++)
        {
            array.push_back(Glib::ustring(session->error_description[i]));
        }
    }
    return array;
}


const char* XenServer::getErrorMessage(xen_session* session)
{
    if (!session || session->ok || !session->error_description_count || !session->error_description)
    {
        return NULL;
    }
    else if (!strcmp(session->error_description[0], ERROR_TRANSPORT_FAULT))
    {
        return gettext("Possibly the server is down or the hostname is incorrect.");
    }
    else if (!strcmp(session->error_description[0], ERROR_SESSION_AUTHENTICATION_FAILED))
    {
        return gettext("The credentials are incorrect.");
    }
    else if (!strcmp(session->error_description[0], ERROR_SERVER_FAULT))
    {
        return gettext("The server doesn't appear to be a XenServer.");
    }
    else
    {
        return NULL;
    }
}


void XenServer::setError(xen_session* session, const char* error, ...)
{
    if (!session)
    {
        return;
    }
    std::vector<const char*> list;
    va_list v;
    va_start(v, error);
    while (error)
    {
        list.push_back(error);
        error = va_arg(v, const char*);
    }
    va_end(v);
    xen_session_clear_error(session);
    session->ok = 0;
    session->error_description_count = (int)list.size();
    session->error_description = (char**)calloc(list.size() + 1, sizeof(char*));
    if (!session->error_description)
    {
        throw std::bad_alloc();
    }
    for (size_t i = 0; i < list.size(); i++)
    {
        char* s = xen_strdup_(list[i]);
        if (!s)
        {
            throw std::bad_alloc();
        }
        session->error_description[i] = s;
    }
}


void XenServer::setError(xen_session* session, const std::vector<Glib::ustring>& array)
{
    if (!session)
    {
        return;
    }
    xen_session_clear_error(session);
    session->ok = false;
    session->error_description_count = (int)array.size();
    session->error_description = (char**)calloc(array.size() + 1, sizeof(char*));
    if (!session->error_description)
    {
        throw std::bad_alloc();
    }
    for (size_t i = 0; i < array.size(); i++)
    {
        char* s = xen_strdup_(array[i].c_str());
        if (!s)
        {
            throw std::bad_alloc();
        }
        session->error_description[i] = s;
    }
}


bool XenServer::getErrorFromTask(xen_session* session, xen_task task, StringBuffer& buffer, const char* separator)
{
    XenPtr<xen_string_set> errorInfo;
    if (xen_task_get_error_info(session, errorInfo.address(), task))
    {
        if (errorInfo->size)
        {
            if (StartsWith(errorInfo->contents[0], ERROR_SR_BACKEND_FAILURE) && errorInfo->size >= 3)
            {
                if (buffer.len())
                {
                    buffer += separator;
                }
                buffer += errorInfo->contents[2];
                for (size_t i = 3; i < errorInfo->size; i++)
                {
                    if (errorInfo->contents[i] && errorInfo->contents[i][0])
                    {
                        buffer += separator;
                        buffer += errorInfo->contents[i];
                    }
                }
                const char* cur = errorInfo->contents[0] + strlen(ERROR_SR_BACKEND_FAILURE);
                if (*cur == '_')
                {
                    cur++;
                }
                long code = strtol(cur, NULL, 0);
                if (code)
                {
                    buffer += separator;
                    buffer.appendFormat(gettext("(error code %ld)"), code);
                }
            }
            else
            {
                if (buffer.len())
                {
                    buffer += separator;
                }
                buffer += errorInfo->contents[0];
                for (size_t i = 1; i < errorInfo->size; i++)
                {
                    if (errorInfo->contents[i] && errorInfo->contents[i][0])
                    {
                        buffer += separator;
                        buffer += errorInfo->contents[i];
                    }
                }
            }
        }
        return true;
    }
    else
    {
        if (buffer.len())
        {
            buffer += separator;
        }
        buffer += gettext("Error information unavailable from task:");
        getError(session, buffer, separator);
        xen_session_clear_error(session);
        return false;
    }
}


const char* XenServer::getPowerStateText(int state)
{
    switch (state)
    {
    case XEN_VM_POWER_STATE_HALTED:
        return gettext("Halted");
    case XEN_VM_POWER_STATE_PAUSED:
        return gettext("Paused");
    case XEN_VM_POWER_STATE_RUNNING:
        return gettext("Running");
    case XEN_VM_POWER_STATE_SUSPENDED:
        return gettext("Suspended");
    case XEN_VM_POWER_STATE_UNDEFINED:
        return gettext("(undefined)");
    default:
        return gettext("(unknown)");
    }
}


Glib::ustring XenServer::getOs(const xen_vm_guest_metrics_record* record)
{
    Glib::ustring value;
    const char* s1 = find(record->os_version, "name");
    if (s1)
    {
        // os|windir|partition
        const char *s2 = strchr(s1, '|');
        if (s2)
        {
            value.assign(s1, s2 - s1);
        }
        else
        {
            value.assign(s1);
        }
    }
    return value;
}


Glib::ustring XenServer::getIp(const xen_vm_guest_metrics_record* record, int device)
{
    Glib::ustring value;
    if (record && record->networks)
    {
        for (size_t i = 0; i < record->networks->size; i++)
        {
            char *s1 = record->networks->contents[i].key;
            char *s2 = s1;
            int j = (int)strtoul(s1, &s2, 10);
            if (s1 < s2 && j == device && !strcmp(s2, "/ip"))
            {
                value = record->networks->contents[i].val;
                break;
            }
        }
    }
    return value;
}


Glib::ustring XenServer::getIpv6(const xen_vm_guest_metrics_record* record, int device)
{
    Glib::ustring value;
    if (record && record->networks)
    {
        for (size_t i = 0; i < record->networks->size; i++)
        {
            char *s1 = record->networks->contents[i].key;
            char *s2 = s1;
            int j = (int)strtoul(s1, &s2, 10);
            if (s1 < s2 && j == device && !strncmp(s2, "/ipv6/", 6))
            {
                value = record->networks->contents[i].val;
                break;
            }
        }
    }
    return value;
}


const char* XenServer::getParent(const xen_vdi_record* record)
{
    return record ? find(record->sm_config, "vhd-parent") : NULL;
}


Glib::ustring XenServer::getDefaultSr(xen_session* session)
{
    Glib::ustring refid;
    XenPtr<xen_pool_set> ps;
    if (xen_pool_get_all(session, ps.address()))
    {
        for (size_t i = 0; i < ps->size; i++)
        {
            XenRef<xen_sr, xen_sr_free_t> sr;
            if (xen_pool_get_default_sr(session, &sr, ps->contents[i]))
            {
                refid = sr.toString();
                break;
            }
            else
            {
                xen_session_clear_error(session);
            }
        }
    }
    else
    {
        xen_session_clear_error(session);
    }
    return refid;
}


int XenServer::getDefaultSr(xen_session* session, std::vector<Glib::ustring>& array)
{
    int count = 0;
    if (session)
    {
        XenPtr<xen_pool_set> ps;
        if (!xen_pool_get_all(session, ps.address()))
        {
            xen_session_clear_error(session);
            goto done;
        }
        for (size_t i = 0; i < ps->size; i++)
        {
            XenRef<xen_sr, xen_sr_free_t> sr;
            if (xen_pool_get_default_sr(session, &sr, ps->contents[i]))
            {
                array.push_back(sr.toString());
                count++;
            }
            else
            {
                xen_session_clear_error(session);
            }
        }
    }
done:
    return count;
}


bool XenServer::isDefaultSr(xen_session* session, xen_sr sr)
{
    if (session)
    {
        XenPtr<xen_pool_set> ps;
        if (!xen_pool_get_all(session, ps.address()))
        {
            xen_session_clear_error(session);
            goto done;
        }
        for (size_t i = 0; i < ps->size; i++)
        {
            XenRef<xen_sr, xen_sr_free_t> sr2;
            if (xen_pool_get_default_sr(session, &sr2, ps->contents[i]))
            {
                if (sr2.toString() == (const char*)sr)
                {
                    return true;
                }
            }
            else
            {
                xen_session_clear_error(session);
                continue;
            }
        }
    }
done:
    return false;
}


bool XenServer::setDefaultSr(xen_session* session, xen_sr sr)
{
    XenPtr<xen_pool_set> ps;
    if (!xen_pool_get_all(session, ps.address()))
    {
        xen_session_clear_error(session);
        return false;
    }
    if (!ps->size)
    {
        return false;
    }
    for (size_t i = 0; i < ps->size; i++)
    {
        if (!xen_pool_set_default_sr(session, ps->contents[i], sr))
        {
            xen_session_clear_error(session);
            return false;
        }
    }
    return true;
}


int64_t XenServer::getDiskSizeHint(const xen_vm_record *record)
{
    if (record)
    {
        const char* value = find(record->other_config, "disks");
        if (value)
        {
            const char *s1 = strstr(value, " size=");
            if (s1) s1 += 6; else goto done;
            char q = *s1++;
            if (q != '\"' && q != '\'') { q = 0; s1--; }
            const char *s2 = s1;
            int64_t n = strtoul(s1, (char **)&s2, 10);
            if (s1 == s2 || (q && *s2 != q)) goto done;
            return n;
        }
    }
done:
    return 0;
}


const char* XenServer::find(const xen_string_string_map* ss, const char* key)
{
    if (ss && ss->size)
    {
        for (size_t i = 0; i < ss->size; i++)
        {
            if (!strcmp(ss->contents[i].key, key))
            {
                return ss->contents[i].val;
            }
        }
    }
    return NULL;
}


int XenServer::match(const xen_string_string_map* ss, const char* key)
{
    const char* val = find(ss, key);
    if (val)
    {
        return 0;
    }
    return -1;
}


int XenServer::match(const xen_string_string_map* ss, const char* key, const char* value1)
{
    const char* val = find(ss, key);
    if (val)
    {
        if (!strcmp(val, value1))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return -1;
}


int XenServer::match(const xen_string_string_map* ss, const char* key, const char* value1, const char* value2)
{
    const char* val = find(ss, key);
    if (val)
    {
        if (!strcmp(val, value1))
        {
            return 1;
        }
        else if (!strcmp(val, value2))
        {
            return 2;
        }
        else
        {
            return 0;
        }
    }
    return -1;
}


const char* XenServer::getText(enum xen_vbd_type type)
{
    switch (type)
    {
    case XEN_VBD_TYPE_CD: return gettext("CD");
    case XEN_VBD_TYPE_DISK: return gettext("Disk");
    default: return gettext("Unknown");
    }
}


bool XenServer::createVirtualMachine(xen_session* session, const VirtualMachineSpec& spec, xen_vm* vmReturn)
{
    Trace trace("XenServer::createVirtualMachine");

    trace.put("xen_vm_clone(%s,%s)", spec.templateREFID.data(), spec.name.data());

    if (!xen_vm_clone(session, vmReturn, (xen_vm)(char*)spec.templateREFID.data(), (char*)spec.name.data()))
    {
        Logger::instance().error("%s: Vm clone failed.", trace.name().data());
        return false;
    }
    xen_vm& vm = *vmReturn;

    trace.put("xen_vm_set_is_a_template(%s,false)", (char*)vm);

    if (!xen_vm_set_is_a_template(session, vm, false))
    {
        Logger::instance().error("%s: Resetting template flag failed.", trace.name().data());
        return false;
    }

    trace.put("xen_vm_set_name_description(%s,%s)", (char*)vm, spec.desc.data());

    if (!xen_vm_set_name_description(session, vm, (char*)spec.desc.data()))
    {
        Logger::instance().error("%s: Setting description failed.", trace.name().data());
        return false;
    }

    int device = 0;
    for (std::list<HardDiskDriveSpec>::const_iterator iter = spec.hddList.begin(); iter != spec.hddList.end(); iter++)
    {
        if (!createHdd(session, vm, device, *iter))
        {
            return false;
        }
        device++;
    }

    if (device < 3)
    {
        device = 3;
    }

    if (!createCd(session, vm, device, (xen_vdi)(char*)spec.cdREFID.data()))
    {
        return false;
    }

    char disks[] = { "disks" };

    trace.put("xen_vm_remove_from_other_config(%s,%s)", (char*)vm, disks);

    if (!xen_vm_remove_from_other_config(session, vm, disks))
    {
        xen_session_clear_error(session);
    }

    char installrepository[] = { "install-repository" };
    char cdrom[] = { "cdrom" };

    trace.put("xen_vm_add_to_other_config(%s,%s,%s)", (char*)vm, installrepository, cdrom);

    if (!xen_vm_add_to_other_config(session, vm, installrepository, cdrom))
    {
        xen_session_clear_error(session);
    }

    device = 0;

    for (std::list<Glib::ustring>::const_iterator iter = spec.nwList.begin(); iter != spec.nwList.end(); iter++)
    {
        if (!createNic(session, vm, device, (xen_network)(char*)iter->data()))
        {
            return false;
        }
        device++;
    }

    return true;
}


bool XenServer::createHdd(xen_session* session, xen_vm vm, int device, const HardDiskDriveSpec& spec)
{
    Trace trace("XenServer::createHdd", "vm=%s device=%d", (char*)vm, device);

    StringBuffer label;
    if (spec.name.empty())
    {
        label.format(gettext("Hard disk drive %d"), device);
    }
    else
    {
        label.assign(spec.name.data());
    }

    StringBuffer description;
    if (spec.description.empty())
    {
        description.assign(gettext("Created by Anago"));
    }
    else
    {
        description.assign(spec.description.data());
    }

    xen_sr_record_opt srRecordOpt = {0};
    srRecordOpt.u.handle = (xen_sr)spec.srREFID.data();

    xen_vdi_record vdiRecord = {0};
    vdiRecord.name_label = label.ptr();
    vdiRecord.name_description = description.ptr();
    vdiRecord.sr = &srRecordOpt;
    vdiRecord.virtual_size = spec.size;
    vdiRecord.type = XEN_VDI_TYPE_SYSTEM;
    vdiRecord.sharable = false;
    vdiRecord.read_only = false;
    vdiRecord.other_config = xen_string_string_map_alloc(0);

    trace.put("xen_vdi_create(%zu)", spec.size);

    XenRef<xen_vdi, xen_vdi_free_t> vdi;
    bool result = xen_vdi_create(session, &vdi, &vdiRecord);

    xen_string_string_map_free(vdiRecord.other_config);

    if (!result)
    {
        Logger::instance().error("%s: VDI create failed.", trace.name().data());
        return false;
    }

    StringBuffer userdevice;

    xen_vm_record_opt vmRecordOpt = {0};
    vmRecordOpt.u.handle = vm;

    xen_vdi_record_opt vdiRecordOpt = {0};
    vdiRecordOpt.u.handle = vdi;

    xen_vbd_record vbdRecord = {0};
    vbdRecord.vm = &vmRecordOpt;
    vbdRecord.vdi = &vdiRecordOpt;
    vbdRecord.userdevice = userdevice.format("%d", device).ptr();
    vbdRecord.type = XEN_VBD_TYPE_DISK;
    vbdRecord.mode = XEN_VBD_MODE_RW;
    vbdRecord.bootable = true;
    vbdRecord.qos_algorithm_params = xen_string_string_map_alloc(0);
    vbdRecord.other_config = xen_string_string_map_alloc(0);

    trace.put("xen_vbd_create");

    XenRef<xen_vbd, xen_vbd_free_t> vbd;
    result = xen_vbd_create(session, &vbd, &vbdRecord);

    xen_string_string_map_free(vbdRecord.qos_algorithm_params);
    xen_string_string_map_free(vbdRecord.other_config);

    if (!result)
    {
        Logger::instance().error("%s: VBD create failed.", trace.name().data());
        return false;
    }

    trace.put("xen_vm_get_uuid", spec.size);

    char* tmp = NULL;
    if (!xen_vm_get_uuid(session, &tmp, vm))
    {
        Logger::instance().error("%s: VM UUID get failed.", trace.name().data());
        return false;
    }
    StringBuffer uuid;
    uuid = tmp;
    xen_uuid_free(tmp);

    static char vmhint[] = { "vmhint" };

    trace.put("xen_vdi_remove_from_sm_config(vmhint)");

    if (!xen_vdi_remove_from_sm_config(session, vdi, vmhint))
    {
        Logger::instance().error("%s: vmhint remove failed.", trace.name().data());
        return false;
    }

    trace.put("xen_vdi_add_to_sm_config(vmhint)");

    if (!xen_vdi_add_to_sm_config(session, vdi, vmhint, uuid.ptr()))
    {
        Logger::instance().error("%s: vmhint add failed.", trace.name().data());
        return false;
    }

    return true;
}


bool XenServer::createCd(xen_session* session, xen_vm vm, int device, xen_vdi vdi)
{
    Trace trace("XenServer::createCd", "vm=%s device=%d vdi=%s", (char*)vm, device, (char*)vdi);

    StringBuffer userdevice;
    userdevice.format("%d", device);

    xen_vm_record_opt vmRecordOpt = {0};
    vmRecordOpt.u.handle = vm;

    xen_vdi_record_opt vdiRecordOpt = {0};
    vdiRecordOpt.u.handle = vdi;

    xen_vbd_record vbdRecord = {0};
    vbdRecord.vm = &vmRecordOpt;
    vbdRecord.vdi = IS_NULLREF(vdi) ? NULL : &vdiRecordOpt;
    vbdRecord.userdevice = userdevice.ptr();
    vbdRecord.type = XEN_VBD_TYPE_CD;
    vbdRecord.mode = XEN_VBD_MODE_RO;
    vbdRecord.bootable = true;
    vbdRecord.empty = IS_NULLREF(vdi) ? true : false;
    vbdRecord.qos_algorithm_params = xen_string_string_map_alloc(0);
    vbdRecord.other_config = xen_string_string_map_alloc(0);

    trace.put("xen_vbd_create");

    XenRef<xen_vbd, xen_vbd_free_t> vbd;
    bool result = xen_vbd_create(session, &vbd, &vbdRecord);

    xen_string_string_map_free(vbdRecord.qos_algorithm_params);
    xen_string_string_map_free(vbdRecord.other_config);

    if (!result)
    {
        Logger::instance().error("%s: VBD (CD device) create failed.", trace.name().data());
        return false;
    }

    return true;
}


bool XenServer::createNic(xen_session* session, xen_vm vm, int device, xen_network nw)
{
    Trace trace("XenServer::createNic", "vm=%s device=%d nw=%s", (char*)vm, device, (char*)nw);

    StringBuffer deviceString;
    deviceString.format("%d", device);

    xen_vm_record_opt vmRecordOpt = {0};
    vmRecordOpt.u.handle = vm;

    xen_network_record_opt nwRecordOpt = {0};
    nwRecordOpt.u.handle = nw;

    xen_vif_record vifRecord = {0};
    vifRecord.vm = &vmRecordOpt;
    vifRecord.network = &nwRecordOpt;
    vifRecord.device = deviceString.ptr();
    vifRecord.mac_autogenerated = true;
    vifRecord.locking_mode = XEN_VIF_LOCKING_MODE_NETWORK_DEFAULT;
    vifRecord.qos_algorithm_params = xen_string_string_map_alloc(0);
    vifRecord.other_config = xen_string_string_map_alloc(0);
    vifRecord.ipv4_allowed = xen_string_set_alloc(0);
    vifRecord.ipv6_allowed = xen_string_set_alloc(0);

    trace.put("xen_vif_create");

    XenRef<xen_vif, xen_vif_free_t> vif;
    bool result = xen_vif_create(session, &vif, &vifRecord);

    xen_string_string_map_free(vifRecord.qos_algorithm_params);
    xen_string_string_map_free(vifRecord.other_config);
    xen_string_set_free(vifRecord.ipv4_allowed);
    xen_string_set_free(vifRecord.ipv6_allowed);

    if (!result)
    {
        Logger::instance().error("%s: VIF create failed.", trace.name().data());
        return false;
    }

    return true;
}


bool XenServer::attachHdd(xen_session* session, xen_vm vm, const char* userdevice, xen_vdi vdi)
{
    xen_vm_record_opt vmRecordOpt = {0};
    vmRecordOpt.u.handle = vm;
    xen_vdi_record_opt vdiRecordOpt = {0};
    vdiRecordOpt.u.handle = vdi;
    xen_vbd_record vbdRecord = {0};
    vbdRecord.vm = &vmRecordOpt;
    vbdRecord.vdi = &vdiRecordOpt;
    vbdRecord.userdevice = const_cast<char*>(userdevice);
    vbdRecord.type = XEN_VBD_TYPE_DISK;
    vbdRecord.mode = XEN_VBD_MODE_RW;
    vbdRecord.bootable = userdevice && !strcmp(userdevice, "0") ? true : false;
    vbdRecord.qos_algorithm_params = xen_string_string_map_alloc(0);
    vbdRecord.other_config = xen_string_string_map_alloc(0);

    XenRef<xen_vbd, xen_vbd_free_t> vbdRefid;
    bool result = xen_vbd_create(session, &vbdRefid, &vbdRecord);

    xen_string_string_map_free(vbdRecord.qos_algorithm_params);
    xen_string_string_map_free(vbdRecord.other_config);

    return result;
}
