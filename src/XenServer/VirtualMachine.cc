// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "Macros.h"
#include "Session.h"
#include "StorageRepository.h"
#include "VirtualBlockDevice.h"
#include "VirtualDiskImage.h"
#include "VirtualMachine.h"
#include "XenObjectStore.h"
#include "XenRef.h"
#include "XenTask.h"


using namespace hnrt;


RefPtr<VirtualMachine> VirtualMachine::create(Session& session, xen_vm handle, const XenPtr<xen_vm_record>& record)
{
    RefPtr<VirtualMachine> object(new VirtualMachine(session, handle, record));
    session.getStore().add(object);
    return object;
}


VirtualMachine::VirtualMachine(Session& session, xen_vm handle, const XenPtr<xen_vm_record>& record)
    : XenObject(XenObject::VM, session, reinterpret_cast<char*>(handle), record->uuid, record->name_label)
    , _record(record)
{
    _displayStatus = XenServer::getPowerStateText(_record->power_state);
    if (!record->is_a_template &&
        !record->is_control_domain &&
        !record->is_a_snapshot)
    {
        if (record->metrics && !record->metrics->is_record)
        {
            if (!xen_vm_metrics_get_record(session, _metricsRecord.address(), record->metrics->u.handle))
            {
                session.clearError();
            }
        }
        if (record->guest_metrics && !record->guest_metrics->is_record)
        {
            if (!xen_vm_guest_metrics_get_record(session, _guestMetricsRecord.address(), record->guest_metrics->u.handle))
            {
                session.clearError();
            }
        }
    }
}


int VirtualMachine::setBusy(bool value)
{
    int count = XenObject::setBusy(value);
    if (!count)
    {
        setDisplayStatus(XenServer::getPowerStateText(getRecord()->power_state));
    }
    return count;
}


XenPtr<xen_vm_record> VirtualMachine::getRecord()
{
    Glib::Mutex::Lock k(_mutex);
    return XenPtr<xen_vm_record>(_record);
}


void VirtualMachine::setRecord(const XenPtr<xen_vm_record>& record)
{
    xen_vm_power_state prev_power_state;
    if (record)
    {
        Glib::Mutex::Lock k(_mutex);
        prev_power_state = _record->power_state;
        _record = record;
    }
    else
    {
        return;
    }
    XenObject::setName(record->name_label);
    if (record->power_state != prev_power_state)
    {
        emit(POWER_STATE_UPDATED);
        if (!_busyCount)
        {
            setDisplayStatus(XenServer::getPowerStateText(record->power_state));
        }
    }
    emit(RECORD_UPDATED);
}


XenPtr<xen_vm_metrics_record> VirtualMachine::getMetricsRecord()
{
    Glib::Mutex::Lock k(_mutex);
    return XenPtr<xen_vm_metrics_record>(_metricsRecord);
}


void VirtualMachine::setRecord(const XenPtr<xen_vm_metrics_record>& record)
{
    if (record)
    {
        Glib::Mutex::Lock k(_mutex);
        _metricsRecord = record;
    }
    else
    {
        return;
    }
    emit(RECORD_UPDATED);
}


XenPtr<xen_vm_guest_metrics_record> VirtualMachine::getGuestMetricsRecord()
{
    Glib::Mutex::Lock k(_mutex);
    return XenPtr<xen_vm_guest_metrics_record>(_guestMetricsRecord);
}


void VirtualMachine::setRecord(const XenPtr<xen_vm_guest_metrics_record>& record)
{
    if (record)
    {
        Glib::Mutex::Lock k(_mutex);
        _guestMetricsRecord = record;
    }
    else
    {
        return;
    }
    emit(RECORD_UPDATED);
}


int VirtualMachine::getVbds(std::list<RefPtr<VirtualBlockDevice> >& vbds)
{
    XenPtr<xen_vm_record> record = getRecord();
    if (!record->vbds)
    {
        return 0;
    }
    int count = 0;
    for (size_t i = 0; i < record->vbds->size; i++)
    {
        RefPtr<VirtualBlockDevice> vbd = _session.getStore().getVbd(record->vbds->contents[i]);
        if (!vbd)
        {
            continue;
        }
        XenPtr<xen_vbd_record> vbdRecord = vbd->getRecord();
        for (std::list<RefPtr<VirtualBlockDevice> >::iterator iter = vbds.begin();; iter++)
        {
            if (iter == vbds.end())
            {
                vbds.push_back(vbd);
                break;
            }
            else if (vbdRecord->userdevice < (*iter)->getRecord()->userdevice)
            {
                vbds.insert(iter, vbd);
                break;
            }
        }
        count++;
    }
    return count;
}


RefPtr<VirtualBlockDevice> VirtualMachine::getVbd(const RefPtr<VirtualDiskImage>& vdi)
{
    XenPtr<xen_vm_record> record = getRecord();
    if (!record->vbds)
    {
        return RefPtr<VirtualBlockDevice>();
    }
    for (size_t i = 0; i < record->vbds->size; i++)
    {
        RefPtr<VirtualBlockDevice> vbd = _session.getStore().getVbd(record->vbds->contents[i]);
        if (!vbd)
        {
            continue;
        }
        RefPtr<VirtualDiskImage> vdi2 = vbd->getVdi();
        if (vdi2 == vdi)
        {
            return vbd;
        }
    }
    return RefPtr<VirtualBlockDevice>();
}


bool VirtualMachine::setName(const char* label, const char* description)
{
    if (!xen_vm_set_name_label(_session, getXenRef(), (char*)label))
    {
        emit(ERROR);
        return false;
    }

    if (!xen_vm_set_name_description(_session, getXenRef(), (char*)description))
    {
        emit(ERROR);
        return false;
    }

    return true;
}


bool VirtualMachine::setMemory(int64_t staticMin, int64_t staticMax, int64_t dynamicMin, int64_t dynamicMax)
{
    if (!xen_vm_set_memory_limits(_session, getXenRef(), staticMin, staticMax, dynamicMin, dynamicMax))
    {
        emit(ERROR);
        return false;
    }

    return true;
}


bool VirtualMachine::setShadowMemory(double multiplier)
{
    if (!xen_vm_set_hvm_shadow_multiplier(_session, getXenRef(), multiplier))
    {
        emit(ERROR);
        return false;
    }

    return true;
}


bool VirtualMachine::setVcpu(int64_t vcpusMax, int64_t vcpusAtStartup, int coresPerSocket)
{
    if (vcpusMax < 1)
    {
        vcpusMax = 1;
    }

    if (vcpusAtStartup > vcpusMax)
    {
        vcpusAtStartup = vcpusMax;
    }

    int64_t vcpusAtStartupCurrent = getRecord()->vcpus_at_startup;

    if (vcpusMax < vcpusAtStartupCurrent)
    {
        if (!xen_vm_set_vcpus_at_startup(_session, getXenRef(), vcpusAtStartup))
        {
            emit(ERROR);
            return false;
        }
        if (!xen_vm_set_vcpus_max(_session, getXenRef(), vcpusMax))
        {
            emit(ERROR);
            return false;
        }
    }
    else
    {
        if (!xen_vm_set_vcpus_max(_session, getXenRef(), vcpusMax))
        {
            emit(ERROR);
            return false;
        }
        if (!xen_vm_set_vcpus_at_startup(_session, getXenRef(), vcpusAtStartup))
        {
            emit(ERROR);
            return false;
        }
    }

    if (!setCoresPerSocket(coresPerSocket))
    {
        return false;
    }

    return true;
}


#define KEY_CORES_PER_SOCKET "cores-per-socket"


int VirtualMachine::getCoresPerSocket()
{
    int retval = 1;
    XenPtr<xen_vm_record> record = getRecord();
    const char* value = XenServer::find(record->platform, KEY_CORES_PER_SOCKET);
    if (value)
    {
        retval = (int)strtoul(value, NULL, 10);
    }
    return retval;
}


bool VirtualMachine::setCoresPerSocket(int value)
{
    if(!xen_vm_remove_from_platform(_session, getXenRef(), (char*)KEY_CORES_PER_SOCKET))
    {
        emit(ERROR);
        return false;
    }

    if (value > 1)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", value);
        if (!xen_vm_add_to_platform(_session, getXenRef(), (char*)KEY_CORES_PER_SOCKET, buf))
        {
            emit(ERROR);
            return false;
        }
    }

    return true;
}


bool VirtualMachine::start()
{
    XenRef<xen_task, xen_task_free_t> task;
    if (xen_vm_start_async(_session, &task, getXenRef(), false, false))
    {
        setBusy();
        setDisplayStatus(gettext("Starting..."));
        XenTask::create(_session, task, this);
        return true;
    }
    else
    {
        emit(ERROR);
        return false;
    }
}


bool VirtualMachine::shutdown(bool hard)
{
    XenRef<xen_task, xen_task_free_t> task;
    if (hard)
    {
        if (xen_vm_hard_shutdown_async(_session, &task, getXenRef()))
        {
            setBusy();
            setDisplayStatus(gettext("Forcibly shutting down..."));
            XenTask::create(_session, task, this);
            return true;
        }
    }
    else if (xen_vm_shutdown_async(_session, &task, getXenRef()))
    {
        setBusy();
        setDisplayStatus(gettext("Shutting down..."));
        XenTask::create(_session, task, this);
        return true;
    }
    emit(ERROR);
    return false;
}


bool VirtualMachine::reboot(bool hard)
{
    XenRef<xen_task, xen_task_free_t> task;
    if (hard)
    {
        if (xen_vm_hard_reboot_async(_session, &task, getXenRef()))
        {
            setBusy();
            setDisplayStatus(gettext("Forcibly rebooting..."));
            XenTask::create(_session, task, this);
            return true;
        }
    }
    else if (xen_vm_clean_reboot_async(_session, &task, getXenRef()))
    {
        setBusy();
        setDisplayStatus(gettext("Rebooting..."));
        XenTask::create(_session, task, this);
        return true;
    }
    emit(ERROR);
    return false;
}


bool VirtualMachine::suspend()
{
    XenRef<xen_task, xen_task_free_t> task;
    if (xen_vm_suspend_async(_session, &task, getXenRef()))
    {
        setBusy();
        setDisplayStatus(gettext("Suspending..."));
        XenTask::create(_session, task, this);
        return true;
    }
    else
    {
        emit(ERROR);
        return false;
    }
}


bool VirtualMachine::resume()
{
    XenRef<xen_task, xen_task_free_t> task;
    if (xen_vm_resume_async(_session, &task, getXenRef(), false, false))
    {
        setBusy();
        setDisplayStatus(gettext("Resuming..."));
        XenTask::create(_session, task, this);
        return true;
    }
    else
    {
        emit(ERROR);
        return false;
    }
}


bool VirtualMachine::changeCd(xen_vbd vbd, xen_vdi vdi)
{
    XenRef<xen_vdi, xen_vdi_free_t> vdiPrev;
    if (!xen_vbd_get_vdi(_session, &vdiPrev, vbd))
    {
        emit(ERROR);
        return false;
    }
    else if (!vdiPrev.isNull() && !xen_vbd_eject(_session, vbd))
    {
        emit(ERROR);
        return false;
    }

    if (!IS_NULLREF(vdi) && !xen_vbd_insert(_session, vbd, vdi))
    {
        emit(ERROR);
        return false;
    }

    return true;
}


#define KEY_VGA "vga"
#define VAL_VGA_STD "std"


bool VirtualMachine::isStdVga()
{
    return getVga() == VAL_VGA_STD;
}


bool VirtualMachine::setStdVga(bool stdVga)
{
    return setVga(stdVga ? VAL_VGA_STD : NULL);
}


Glib::ustring VirtualMachine::getVga()
{
    Glib::ustring retval;
    XenPtr<xen_vm_record> record = getRecord();
    const char* value = XenServer::find(record->platform, KEY_VGA);
    if (value)
    {
        retval = value;
    }
    return retval;
}


bool VirtualMachine::setVga(const char* value)
{
    if(!xen_vm_remove_from_platform(_session, getXenRef(), (char*)KEY_VGA))
    {
        emit(ERROR);
        return false;
    }

    if (value)
    {
        if (!xen_vm_add_to_platform(_session, getXenRef(), (char*)KEY_VGA, (char*)value))
        {
            emit(ERROR);
            return false;
        }
    }

    return true;
}


#define KEY_VIDEORAM "videoram"


int VirtualMachine::getVideoRam()
{
    int retval = -1;
    XenPtr<xen_vm_record> record = getRecord();
    const char* value = XenServer::find(record->platform, KEY_VIDEORAM);
    if (value)
    {
        retval = (int)strtol(value, NULL, 10);
    }
    return retval;
}


bool VirtualMachine::setVideoRam(int value)
{
    if(!xen_vm_remove_from_platform(_session, getXenRef(), (char*)KEY_VIDEORAM))
    {
        emit(ERROR);
        return false;
    }

    if (value > 0)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", value);
        if (!xen_vm_add_to_platform(_session, getXenRef(), (char*)KEY_VIDEORAM, buf))
        {
            emit(ERROR);
            return false;
        }
    }

    return true;
}


Glib::ustring VirtualMachine::getPrimarySr()
{
    Glib::ustring refid;
    std::list<RefPtr<VirtualBlockDevice> > vbds;
    if (getVbds(vbds))
    {
        for (std::list<RefPtr<VirtualBlockDevice> >::iterator iter = vbds.begin(); iter != vbds.end(); iter++)
        {
            RefPtr<VirtualDiskImage> vdi = (*iter)->getVdi();
            if (!vdi)
            {
                continue;
            }
            RefPtr<StorageRepository> sr = vdi->getSr();
            if (sr)
            {
                refid = sr->getREFID();
                break;
            }
        }
    }
    return refid;
}


bool VirtualMachine::clone(const char* name)
{
    XenRef<xen_task, xen_task_free_t> task;
    if (xen_vm_clone_async(_session, &task, getXenRef(), (char*)name))
    {
        setBusy();
        setDisplayStatus(gettext("Cloning..."));
        XenTask::create(_session, task, this);
        return true;
    }
    else
    {
        emit(ERROR);
        return false;
    }
}


bool VirtualMachine::copy(const char* name, xen_sr sr)
{
    XenRef<xen_task, xen_task_free_t> task;
    if (xen_vm_copy_async(_session, &task, getXenRef(), (char*)name, sr))
    {
        setBusy();
        setDisplayStatus(gettext("Copying..."));
        XenTask::create(_session, task, this);
        return true;
    }
    else
    {
        emit(ERROR);
        return false;
    }
}


bool VirtualMachine::destroy()
{
    XenRef<xen_task, xen_task_free_t> task;
    if (xen_vm_destroy_async(_session, &task, getXenRef()))
    {
        setDisplayStatus(gettext("Being destroyed..."));
        XenTask::create(_session, task, this);
        return true;
    }
    else
    {
        emit(ERROR);
        return false;
    }
}


Glib::ustring VirtualMachine::getConsoleLocation()
{
    Glib::ustring location;
    XenPtr<xen_console_set> conSet;
    if (xen_vm_get_consoles(_session, conSet.address(), getXenRef()))
    {
        for (size_t i = 0; i < conSet->size; i++)
        {
            XenPtr<xen_console_record> record;
            if (xen_console_get_record(_session, record.address(), conSet->contents[i]))
            {
                if (record->protocol == XEN_CONSOLE_PROTOCOL_RFB)
                {
                    location = record->location;
                    break;
                }
            }
            else
            {
                emit(ERROR);
                break;
            }
        }
    }
    else
    {
        emit(ERROR);
    }
    return location;
}
