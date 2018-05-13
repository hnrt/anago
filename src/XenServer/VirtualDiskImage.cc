// Copyright (C) 2012-2018 Hideaki Narita


#include <libintl.h>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Session.h"
#include "StorageRepository.h"
#include "VirtualBlockDevice.h"
#include "VirtualDiskImage.h"
#include "VirtualMachine.h"
#include "XenObjectStore.h"
#include "XenTask.h"


using namespace hnrt;


RefPtr<VirtualDiskImage> VirtualDiskImage::create(Session& session, xen_vdi handle, const XenPtr<xen_vdi_record>& record)
{
    RefPtr<VirtualDiskImage> object(new VirtualDiskImage(session, handle, record));
    session.getStore().add(object);
    return object;
}


VirtualDiskImage::VirtualDiskImage(Session& session, xen_vdi handle, const XenPtr<xen_vdi_record>& record)
    : XenObject(XenObject::VDI, session, handle, record->uuid, record->name_label)
    , _record(record)
{
    Trace trace(this, "VDI::ctor");
}


VirtualDiskImage::~VirtualDiskImage()
{
    Trace trace(this, "VDI::dtor");
}


XenPtr<xen_vdi_record> VirtualDiskImage::getRecord() const
{
    Glib::Mutex::Lock lock(const_cast<VirtualDiskImage*>(this)->_mutex);
    return _record;
}


void VirtualDiskImage::setRecord(const XenPtr<xen_vdi_record>& record)
{
    if (record)
    {
        Glib::Mutex::Lock lock(_mutex);
        _record = record;
    }
    else
    {
        return;
    }
    XenObject::setName(record->name_label);
    emit(RECORD_UPDATED);
}


RefPtr<StorageRepository> VirtualDiskImage::getSr() const
{
    return _session.getStore().getSr(getRecord()->sr);
}


RefPtr<VirtualMachine> VirtualDiskImage::getVm(size_t index) const
{
    XenPtr<xen_vdi_record> vdiRecord = getRecord();
    if (vdiRecord->vbds && index < vdiRecord->vbds->size)
    {
        RefPtr<VirtualBlockDevice> vbd = _session.getStore().getVbd(vdiRecord->vbds->contents[index]);
        if (vbd)
        {
            XenPtr<xen_vbd_record> vbdRecord = vbd->getRecord();
            if (vbdRecord)
            {
                if (vbdRecord->type == XEN_VBD_TYPE_DISK)
                {
                    return _session.getStore().getVm(vbdRecord->vm);
                }
            }
        }
    }
    return RefPtr<VirtualMachine>();
}


bool VirtualDiskImage::setName(const char* label, const char* description)
{
    if (!xen_vdi_set_name_label(_session, _handle, (char*)label))
    {
        emit(ERROR);
        return false;
    }

    if (!xen_vdi_set_name_description(_session, _handle, (char*)description))
    {
        emit(ERROR);
        return false;
    }

    return true;
}


bool VirtualDiskImage::destroy()
{
    setBusy(true);
    XenRef<xen_task, xen_task_free_t> task;
    if (xen_vdi_destroy_async(_session, &task, _handle))
    {
        XenTask::create(_session, task, this,
                        StringBuffer().format(gettext("Failed to destroy VDI \"%s\".\n"), getName().c_str()));
        return true;
    }
    else
    {
        setBusy(false);
        emit(ERROR);
        return false;
    }
}


bool VirtualDiskImage::resize(int64_t value)
{
    setBusy(true);
    XenRef<xen_task, xen_task_free_t> task;
    if (xen_vdi_resize_async(_session, &task, _handle, value))
    {
        XenTask::create(_session, task, this,
                        StringBuffer().format(gettext("Failed to resize VDI \"%s\".\n"), getName().c_str()));
        return true;
    }
    else
    {
        setBusy(false);
        emit(ERROR);
        return false;
    }
}
