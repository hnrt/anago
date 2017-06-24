// Copyright (C) 2012-2017 Hideaki Narita


#include "Logger/Trace.h"
#include "Network.h"
#include "Session.h"
#include "VirtualInterface.h"
#include "VirtualMachine.h"
#include "XenObjectStore.h"


using namespace hnrt;


RefPtr<Network> Network::create(Session& session, xen_network handle, const XenPtr<xen_network_record>& record)
{
    RefPtr<Network> object(new Network(session, handle, record));
    session.getStore().add(object);
    return object;
}


Network::Network(Session& session, xen_network handle, const XenPtr<xen_network_record>& record)
    : XenObject(XenObject::NETWORK, session, handle, record->uuid, record->name_label)
    , _record(record)
{
    Trace trace(this, "NETWORK::ctor");
}


Network::~Network()
{
    Trace trace(this, "NETWORK::dtor");
}


XenPtr<xen_network_record> Network::getRecord() const
{
    Glib::Mutex::Lock lock(const_cast<Network*>(this)->_mutex);
    return _record;
}


void Network::setRecord(const XenPtr<xen_network_record>& record)
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


bool Network::setName(const char* label, const char* description)
{
    if (!xen_network_set_name_label(_session, _handle, (char*)label))
    {
        emit(ERROR);
        return false;
    }

    if (!xen_network_set_name_description(_session, _handle, (char*)description))
    {
        emit(ERROR);
        return false;
    }

    return true;
}


RefPtr<VirtualMachine> Network::getVm(size_t index) const
{
    XenPtr<xen_network_record> record = getRecord();
    if (record->vifs && index < record->vifs->size)
    {
        RefPtr<VirtualInterface> vif = _session.getStore().getVif(record->vifs->contents[index]);
        if (vif)
        {
            XenPtr<xen_vif_record> vifRecord = vif->getRecord();
            if (vifRecord)
            {
                return _session.getStore().getVm(vifRecord->vm);
            }
        }
    }
    return RefPtr<VirtualMachine>();
}


bool Network::isHostInternalManagement() const
{
    return XenServer::match(getRecord()->other_config, "is_host_internal_management_network", "true") == 1;
}
