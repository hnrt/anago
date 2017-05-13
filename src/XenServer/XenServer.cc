// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Util/Base64.h"
#include "Util/Scrambler.h"
#include "Util/Util.h"
#include "CifsSpec.h"
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

    trace.put("xen_vm_clone(%s,%s)", spec.templateREFID.c_str(), spec.name.c_str());

    if (!xen_vm_clone(session, vmReturn, (xen_vm)spec.templateREFID.c_str(), (char*)spec.name.c_str()))
    {
        Logger::instance().error("%s: Vm clone failed.", trace.name().c_str());
        return false;
    }
    xen_vm& vm = *vmReturn;

    trace.put("xen_vm_set_is_a_template(%s,false)", reinterpret_cast<char*>(vm));

    if (!xen_vm_set_is_a_template(session, vm, false))
    {
        Logger::instance().error("%s: Resetting template flag failed.", trace.name().c_str());
        return false;
    }

    trace.put("xen_vm_set_name_description(%s,%s)", reinterpret_cast<char*>(vm), spec.desc.c_str());

    if (!xen_vm_set_name_description(session, vm, (char*)spec.desc.c_str()))
    {
        Logger::instance().error("%s: Setting description failed.", trace.name().c_str());
        return false;
    }

    StringBuffer device;

    int deviceNo = 0;
    for (std::list<HardDiskDriveSpec>::const_iterator iter = spec.hddList.begin(); iter != spec.hddList.end(); iter++)
    {
        device.format("%d", deviceNo);
        if (!createHdd(session, vm, device, *iter))
        {
            return false;
        }
        deviceNo++;
    }

    if (deviceNo < 3)
    {
        deviceNo = 3;
    }

    device.format("%d", deviceNo);
    if (!createCd(session, vm, device, (xen_vdi)spec.cdREFID.c_str()))
    {
        return false;
    }

    char disks[] = { "disks" };

    trace.put("xen_vm_remove_from_other_config(%s,%s)", reinterpret_cast<char*>(vm), disks);

    if (!xen_vm_remove_from_other_config(session, vm, disks))
    {
        xen_session_clear_error(session);
    }

    char installrepository[] = { "install-repository" };
    char cdrom[] = { "cdrom" };

    trace.put("xen_vm_add_to_other_config(%s,%s,%s)", reinterpret_cast<char*>(vm), installrepository, cdrom);

    if (!xen_vm_add_to_other_config(session, vm, installrepository, cdrom))
    {
        xen_session_clear_error(session);
    }

    deviceNo = 0;

    for (std::list<Glib::ustring>::const_iterator iter = spec.nwList.begin(); iter != spec.nwList.end(); iter++)
    {
        device.format("%d", deviceNo);
        if (!createNic(session, vm, device, (xen_network)iter->c_str()))
        {
            return false;
        }
        deviceNo++;
    }

    return true;
}


bool XenServer::createHdd(xen_session* session, xen_vm vm, const char* userdevice, const HardDiskDriveSpec& spec)
{
    Trace trace("XenServer::createHdd");

    XenRef<xen_vdi, xen_vdi_free_t> vdi;
    if (!createVdi(session, spec, &vdi))
    {
        return false;
    }

    XenRef<xen_vbd, xen_vbd_free_t> vbd;
    if (!createVbd(session, vm, userdevice, vdi, XEN_VBD_TYPE_DISK, XEN_VBD_MODE_RW, true, &vbd))
    {
        return false;
    }

    if (!setVmHintToVdi(session, vdi, vm))
    {
        return false;
    }

    return true;
}


bool XenServer::attachHdd(xen_session* session, xen_vm vm, const char* userdevice, xen_vdi vdi)
{
    Trace trace("XenServer::attachHdd");

    XenRef<xen_vbd, xen_vbd_free_t> vbd;
    if (!createVbd(session, vm, userdevice, vdi, XEN_VBD_TYPE_DISK, XEN_VBD_MODE_RW, !strcmp(userdevice, "0"), &vbd))
    {
        return false;
    }

    if (!setVmHintToVdi(session, vdi, vm))
    {
        return false;
    }

    return true;
}


bool XenServer::createCd(xen_session* session, xen_vm vm, const char* userdevice, xen_vdi vdi)
{
    XenRef<xen_vbd, xen_vbd_free_t> vbd;
    return createVbd(session, vm, userdevice, vdi, XEN_VBD_TYPE_CD, XEN_VBD_MODE_RO, true, &vbd);
}


bool XenServer::attachCd(xen_session* session, xen_vm vm, const char* userdevice)
{
    XenRef<xen_vbd, xen_vbd_free_t> vbd;
    return createVbd(session, vm, userdevice, NULLREF, XEN_VBD_TYPE_CD, XEN_VBD_MODE_RO, true, &vbd);
}


bool XenServer::createNic(xen_session* session, xen_vm vm, const char* device, xen_network nw)
{
    XenRef<xen_vif, xen_vif_free_t> vif;
    return createVif(session, vm, device, nw, &vif);
}


bool XenServer::createVdi(xen_session* session, const HardDiskDriveSpec& spec, xen_vdi* vdiReturn)
{
    Trace trace("XenServer::createVdi");

    xen_sr_record_opt srRecordOpt = {0};
    srRecordOpt.u.handle = (xen_sr)spec.srREFID.c_str();

    xen_vdi_record vdiRecord = {0};
    vdiRecord.name_label = const_cast<char*>(spec.label.c_str());
    vdiRecord.name_description = const_cast<char*>(spec.description.c_str());
    vdiRecord.sr = &srRecordOpt;
    vdiRecord.virtual_size = spec.size;
    vdiRecord.type = XEN_VDI_TYPE_SYSTEM;
    vdiRecord.sharable = spec.sharable;
    vdiRecord.read_only = spec.readonly;
    vdiRecord.other_config = xen_string_string_map_alloc(0);

    trace.put("xen_vdi_create(sr=%s size=%'zu label=\"%s\" desc=\"%s\" sharable=%s readonly=%s)",
              spec.srREFID.c_str(),
              spec.size,
              spec.label.c_str(),
              spec.description.c_str(),
              spec.sharable ? "true" : "false",
              spec.readonly ? "true" : "false");

    bool result = xen_vdi_create(session, vdiReturn, &vdiRecord);
    if (result)
    {
        trace.put("vdi=%s", reinterpret_cast<char*>(*vdiReturn));
    }
    else
    {
        Logger::instance().error("%s: VDI create failed. sr=%s size=%'zu", trace.name().c_str(), spec.srREFID.c_str(), spec.size);
    }

    xen_string_string_map_free(vdiRecord.other_config);

    return result;
}


bool XenServer::setVmHintToVdi(xen_session* session, xen_vdi vdi, xen_vm vm)
{
    Trace trace("XenServer::setVmHintToVdi");

    trace.put("xen_vm_get_uuid");

    char* tmp = NULL;
    if (!xen_vm_get_uuid(session, &tmp, vm))
    {
        Logger::instance().error("%s: VM UUID get failed.", trace.name().c_str());
        return false;
    }
    StringBuffer uuid;
    uuid = tmp;
    xen_uuid_free(tmp);

    static char vmhint[] = { "vmhint" };

    trace.put("xen_vdi_remove_from_sm_config(vmhint)");

    if (!xen_vdi_remove_from_sm_config(session, vdi, vmhint))
    {
        Logger::instance().error("%s: vmhint remove failed.", trace.name().c_str());
        return false;
    }

    trace.put("xen_vdi_add_to_sm_config(vmhint)");

    if (!xen_vdi_add_to_sm_config(session, vdi, vmhint, uuid.ptr()))
    {
        Logger::instance().error("%s: vmhint add failed.", trace.name().c_str());
        return false;
    }

    return true;
}


bool XenServer::createVbd(xen_session* session, xen_vm vm, const char* userdevice, xen_vdi vdi, enum xen_vbd_type type, enum xen_vbd_mode mode, bool bootable, xen_vbd* vbdReturn)
{
    Trace trace("XenServer::createVbd");

    xen_vm_record_opt vmRecordOpt = {0};
    vmRecordOpt.u.handle = vm;

    xen_vdi_record_opt vdiRecordOpt = {0};
    vdiRecordOpt.u.handle = vdi;

    xen_vbd_record vbdRecord = {0};
    vbdRecord.vm = &vmRecordOpt;
    vbdRecord.vdi = IS_NULLREF(vdi) ? NULL : &vdiRecordOpt;
    vbdRecord.userdevice = const_cast<char*>(userdevice);
    vbdRecord.type = type;
    vbdRecord.mode = mode;
    vbdRecord.bootable = bootable;
    vbdRecord.empty = IS_NULLREF(vdi) ? true : false;
    vbdRecord.qos_algorithm_params = xen_string_string_map_alloc(0);
    vbdRecord.other_config = xen_string_string_map_alloc(0);

    trace.put("xen_vbd_create(vm=%s userdevice=%s vdi=%s type=%s mode=%s bootable=%s)",
              reinterpret_cast<char*>(vm),
              userdevice,
              reinterpret_cast<char*>(vdi),
              xen_vbd_type_to_string(type),
              xen_vbd_mode_to_string(mode),
              bootable ? "true" : "false");

    bool result = xen_vbd_create(session, vbdReturn, &vbdRecord);
    if (result)
    {
        trace.put("vbd=%s", reinterpret_cast<char*>(*vbdReturn));
    }
    else
    {
        Logger::instance().error("%s: VBD create failed. vm=%s userdevice=%d vdi=%s type=%s mode=%s bootable=%s",
                                 trace.name().c_str(),
                                 reinterpret_cast<char*>(vm),
                                 userdevice,
                                 reinterpret_cast<char*>(vdi),
                                 xen_vbd_type_to_string(type),
                                 xen_vbd_mode_to_string(mode),
                                 bootable ? "true" : "false");
    }

    xen_string_string_map_free(vbdRecord.qos_algorithm_params);
    xen_string_string_map_free(vbdRecord.other_config);


    return result;
}


bool XenServer::createVif(xen_session* session, xen_vm vm, const char* device, xen_network nw, xen_vif* vifReturn)
{
    Trace trace("XenServer::createVif");

    xen_vm_record_opt vmRecordOpt = {0};
    vmRecordOpt.u.handle = vm;

    xen_network_record_opt nwRecordOpt = {0};
    nwRecordOpt.u.handle = nw;

    xen_vif_record vifRecord = {0};
    vifRecord.vm = &vmRecordOpt;
    vifRecord.network = &nwRecordOpt;
    vifRecord.device = const_cast<char*>(device);
    vifRecord.mac_autogenerated = true;
    vifRecord.locking_mode = XEN_VIF_LOCKING_MODE_NETWORK_DEFAULT;
    vifRecord.qos_algorithm_params = xen_string_string_map_alloc(0);
    vifRecord.other_config = xen_string_string_map_alloc(0);
    vifRecord.ipv4_allowed = xen_string_set_alloc(0);
    vifRecord.ipv6_allowed = xen_string_set_alloc(0);

    trace.put("xen_vif_create(vm=%s device=%s nw=%s)",
              reinterpret_cast<char*>(vm),
              device,
              reinterpret_cast<char*>(nw));

    bool result = xen_vif_create(session, vifReturn, &vifRecord);
    if (result)
    {
        trace.put("vif=%s", reinterpret_cast<char*>(*vifReturn));
    }
    else
    {
        Logger::instance().error("%s: VIF create failed. vm=%s device=%s nw=%",
                                 trace.name().c_str(),
                                 reinterpret_cast<char*>(vm),
                                 device,
                                 reinterpret_cast<char*>(nw));
    }

    xen_string_string_map_free(vifRecord.qos_algorithm_params);
    xen_string_string_map_free(vifRecord.other_config);
    xen_string_set_free(vifRecord.ipv4_allowed);
    xen_string_set_free(vifRecord.ipv6_allowed);

    return result;
}


bool XenServer::createSnapshot(xen_session* session, xen_vm vm)
{
    XenPtr<xen_vm_record> record;
    if (!xen_vm_get_record(session, record.address(), vm))
    {
        return false;
    }

    StringBuffer basename;
    basename = record->name_label;

    time_t currentTime = time(NULL);
    struct tm localTime = { 0 };
    localtime_r(&currentTime, &localTime);
    StringBuffer name;
    name.format("%s_snapshot_%04d%02d%02d_%02d%02d%02d",
                basename.str(),
                localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday, localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

    XenRef<xen_vm, xen_vm_free_t> result;
    if (record->power_state == XEN_VM_POWER_STATE_HALTED)
    {
        if (!xen_vm_snapshot(session, &result, vm, name.ptr()))
        {
            return false;
        }
    }
    else if (!xen_vm_snapshot_with_quiesce(session, &result, vm, name.ptr()))
    {
        if (!xen_vm_snapshot(session, &result, vm, name.ptr()))
        {
            return false;
        }
    }

    if (!xen_vm_get_record(session, record.address(), result))
    {
        xen_session_clear_error(session);
        return true;
    }

    time_t snapshotTime = record->snapshot_time - timezone;
    localtime_r(&snapshotTime, &localTime);
    name.format("%s_snapshot_%04d%02d%02d_%02d%02d%02d",
                basename.str(),
                localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday, localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
    if (!xen_vm_set_name_label(session, result, name.ptr()))
    {
        xen_session_clear_error(session);
        return true;
    }

    return true;
}


bool XenServer::addCifs(xen_session* session, xen_host host, const CifsSpec& spec, xen_sr* srReturn)
{
    Trace trace("XenServer::addCifs");

    bool result;

    XenRef<xen_secret, xen_secret_free_t> secret;

    xen_secret_record secretRecord = {0};
    Base64Decoder d1(spec.password.c_str());
    Descrambler d2(d1.getValue(), d1.getLength());
    secretRecord.value = const_cast<char*>(reinterpret_cast<const char*>(d2.getValue()));
    secretRecord.other_config = xen_string_string_map_alloc(0);
    trace.put("xen_secret_create");
    result = xen_secret_create(session, &secret, &secretRecord);
    xen_string_string_map_free(secretRecord.other_config);
    if (!result)
    {
        Logger::instance().error("%s: xen_secret_create failed.", trace.name().c_str());
        return false;
    }

    char* tmp = NULL;
    trace.put("xen_secret_get_uuid");
    if (!xen_secret_get_uuid(session, &tmp, secret))
    {
        Logger::instance().error("%s: Secret UUID get failed.", trace.name().c_str());
        return false;
    }
    StringBuffer uuid;
    uuid = tmp;
    xen_uuid_free(tmp);

    xen_string_string_map *deviceConfig = xen_string_string_map_alloc(4);
    deviceConfig->contents[0].key = xen_strdup_("type");
    deviceConfig->contents[0].val = xen_strdup_("cifs");
    deviceConfig->contents[1].key = xen_strdup_("location");
    deviceConfig->contents[1].val = xen_strdup_(spec.location.c_str());
    deviceConfig->contents[2].key = xen_strdup_("cifspassword_secret");
    deviceConfig->contents[2].val = xen_strdup_(uuid);
    deviceConfig->contents[3].key = xen_strdup_("username");
    deviceConfig->contents[3].val = xen_strdup_(spec.username.c_str());
    int64_t size = 0;
    xen_string_string_map *smConfig = xen_string_string_map_alloc(1);
    smConfig->contents[0].key = xen_strdup_("iso_type");
    smConfig->contents[0].val = xen_strdup_("cifs");
    trace.put("xen_sr_create");
    result = xen_sr_create(session, srReturn, host, deviceConfig, size,
                           (char *)spec.label.c_str(), (char *)spec.description.c_str(),
                           (char *)"iso", (char *)"iso", true, smConfig);
    xen_string_string_map_free(deviceConfig);
    xen_string_string_map_free(smConfig);
    if (!result)
    {
        Logger::instance().error("%s: xen_sr_create(cifs:%s) failed.", trace.name().c_str(), spec.location.c_str());
        return false;
    }

    return true;
}
