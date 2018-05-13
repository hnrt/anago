// Copyright (C) 2012-2018 Hideaki Narita


#include "Logger/Trace.h"
#include "PhysicalInterface.h"
#include "Session.h"
#include "XenObjectStore.h"


using namespace hnrt;


RefPtr<PhysicalInterface> PhysicalInterface::create(Session& session, xen_pif handle, const XenPtr<xen_pif_record>& record)
{
    RefPtr<PhysicalInterface> object(new PhysicalInterface(session, handle, record));
    session.getStore().add(object);
    return object;
}


PhysicalInterface::PhysicalInterface(Session& session, xen_pif handle, const XenPtr<xen_pif_record>& record)
    : XenObject(XenObject::PIF, session, handle, record->uuid, "PIF")
    , _record(record)
{
    Trace trace(this, "PIF::ctor");
}


PhysicalInterface::~PhysicalInterface()
{
    Trace trace(this, "PIF::dtor");
}


XenPtr<xen_pif_record> PhysicalInterface::getRecord() const
{
    Glib::Mutex::Lock lock(const_cast<PhysicalInterface*>(this)->_mutex);
    return XenPtr<xen_pif_record>(_record);
}


void PhysicalInterface::setRecord(const XenPtr<xen_pif_record>& record)
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
