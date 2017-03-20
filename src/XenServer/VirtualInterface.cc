// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Session.h"
#include "VirtualInterface.h"
#include "VirtualMachine.h"
#include "XenObjectStore.h"


using namespace hnrt;


RefPtr<VirtualInterface> VirtualInterface::create(Session& session, xen_vif handle, const XenPtr<xen_vif_record>& record)
{
    RefPtr<VirtualInterface> object(new VirtualInterface(session, handle, record));
    session.getStore().add(object);
    return object;
}


VirtualInterface::VirtualInterface(Session& session, xen_vif handle, const XenPtr<xen_vif_record>& record)
    : XenObject(XenObject::VIF, session, handle, record->uuid, "VIF")
    , _record(record)
{
    Trace trace(StringBuffer().format("VIF@%zx::ctor", this));
}


VirtualInterface::~VirtualInterface()
{
    Trace trace(StringBuffer().format("VIF@%zx::dtor", this));
}


XenPtr<xen_vif_record> VirtualInterface::getRecord() const
{
    Glib::Mutex::Lock lock(const_cast<VirtualInterface*>(this)->_mutex);
    return _record;
}


void VirtualInterface::setRecord(const XenPtr<xen_vif_record>& record)
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


RefPtr<VirtualMachine> VirtualInterface::getVm() const
{
    return _session.getStore().getVm(getRecord()->vm);
}


Glib::ustring VirtualInterface::getDeviceName() const
{
    return Glib::ustring::compose(gettext("Network %1"), getRecord()->device);
}
