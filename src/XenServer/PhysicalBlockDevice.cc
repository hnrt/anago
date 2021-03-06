// Copyright (C) 2012-2018 Hideaki Narita


#include "Logger/Trace.h"
#include "PhysicalBlockDevice.h"
#include "Session.h"
#include "StorageRepository.h"
#include "XenObjectStore.h"


using namespace hnrt;


RefPtr<PhysicalBlockDevice> PhysicalBlockDevice::create(Session& session, xen_pbd handle, const XenPtr<xen_pbd_record>& record)
{
    RefPtr<PhysicalBlockDevice> object(new PhysicalBlockDevice(session, handle, record));
    session.getStore().add(object);
    return object;
}


PhysicalBlockDevice::PhysicalBlockDevice(Session& session, xen_pbd handle, const XenPtr<xen_pbd_record>& record)
    : XenObject(XenObject::PBD, session, reinterpret_cast<char*>(handle), record->uuid, "PBD")
    , _record(record)
{
    Trace trace(this, "PBD::ctor");
}


PhysicalBlockDevice::~PhysicalBlockDevice()
{
    Trace trace(this, "PBD::dtor");
}


XenPtr<xen_pbd_record> PhysicalBlockDevice::getRecord()
{
    Glib::Mutex::Lock k(_mutex);
    return _record;
}


void PhysicalBlockDevice::setRecord(const XenPtr<xen_pbd_record>& record)
{
    if (record)
    {
        Glib::Mutex::Lock k(_mutex);
        _record = record;
    }
    else
    {
        return;
    }
    emit(RECORD_UPDATED);
}


RefPtr<StorageRepository> PhysicalBlockDevice::getSr()
{
    XenPtr<xen_pbd_record> record = getRecord();
    return _session.getStore().getSr(record->sr);
}
