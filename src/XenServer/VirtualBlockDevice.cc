// Copyright (C) 2012-2018 Hideaki Narita


#include <libintl.h>
#include "Logger/Trace.h"
#include "Session.h"
#include "VirtualBlockDevice.h"
#include "VirtualDiskImage.h"
#include "VirtualMachine.h"
#include "XenObjectStore.h"


using namespace hnrt;


static const char* GetVbdTypeText(enum xen_vbd_type type)
{
    switch (type)
    {
    case XEN_VBD_TYPE_CD:
        return gettext("CD");
    case XEN_VBD_TYPE_DISK:
        return gettext("Disk");
    default:
        return gettext("(unknown)");
    }
}


RefPtr<VirtualBlockDevice> VirtualBlockDevice::create(Session& session, xen_vbd handle, const XenPtr<xen_vbd_record>& record)
{
    RefPtr<VirtualBlockDevice> object(new VirtualBlockDevice(session, handle, record));
    session.getStore().add(object);
    return object;
}


VirtualBlockDevice::VirtualBlockDevice(Session& session, xen_vbd handle, const XenPtr<xen_vbd_record>& record)
    : XenObject(XenObject::VBD, session, handle, record->uuid, "VBD")
    , _record(record)
{
    Trace trace(this, "VBD::ctor");
}


VirtualBlockDevice::~VirtualBlockDevice()
{
    Trace trace(this, "VBD::dtor");
}


XenPtr<xen_vbd_record> VirtualBlockDevice::getRecord() const
{
    Glib::Mutex::Lock lock(const_cast<VirtualBlockDevice*>(this)->_mutex);
    return _record;
}


void VirtualBlockDevice::setRecord(const XenPtr<xen_vbd_record>& record)
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
    emit(RECORD_UPDATED);
}


RefPtr<VirtualMachine> VirtualBlockDevice::getVm() const
{
    return _session.getStore().getVm(getRecord()->vm);
}


RefPtr<VirtualDiskImage> VirtualBlockDevice::getVdi() const
{
    return _session.getStore().getVdi(getRecord()->vdi);
}


Glib::ustring VirtualBlockDevice::getUserdevice() const
{
    return Glib::ustring(getRecord()->userdevice);
}


Glib::ustring VirtualBlockDevice::getDeviceName() const
{
    XenPtr<xen_vbd_record> record = getRecord();
    return Glib::ustring::compose("%1 %2", GetVbdTypeText(record->type), record->userdevice);
}
