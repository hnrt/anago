// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <stdio.h>
#include <string.h>
#include "Logger/Trace.h"
#include "Util/Util.h"
#include "PhysicalBlockDevice.h"
#include "Session.h"
#include "StorageRepository.h"
#include "XenObjectStore.h"


using namespace hnrt;


inline StorageRepository::SubType GetSubType(const char* value)
{
    if (!value || !*value)
    {
        return StorageRepository::NONE;
    }
    else if (!strcmp(value, "iso"))
    {
        return StorageRepository::ISO;
    }
    else if (!strcmp(value, "udev"))
    {
        return StorageRepository::DEV;
    }
    else
    {
        return StorageRepository::USR;
    }
}


RefPtr<StorageRepository> StorageRepository::create(Session& session, xen_sr handle, const XenPtr<xen_sr_record>& record)
{
    RefPtr<StorageRepository> object(new StorageRepository(session, handle, record));
    session.getStore().add(object);
    object->setStatus();
    return object;
}


StorageRepository::StorageRepository(Session& session, xen_sr handle, const XenPtr<xen_sr_record>& record)
    : XenObject(XenObject::SR, session, handle, record->uuid, record->name_label)
    , _subType(GetSubType(record->type))
    , _record(record)
{
    Trace trace(StringBuffer().format("VM@%zx::ctor", this));
}


StorageRepository::~StorageRepository()
{
    Trace trace(StringBuffer().format("VM@%zx::dtor", this));
}


int StorageRepository::setBusy(bool value)
{
    int count = XenObject::setBusy(value);
    if (!count)
    {
        setStatus();
    }
    return count;
}


XenPtr<xen_sr_record> StorageRepository::getRecord() const
{
    Glib::Mutex::Lock lock(const_cast<StorageRepository*>(this)->_mutex);
    return _record;
}


void StorageRepository::setRecord(const XenPtr<xen_sr_record>& record)
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
    if (!_busyCount)
    {
        setStatus();
    }
    emit(RECORD_UPDATED);
}


RefPtr<PhysicalBlockDevice> StorageRepository::getPbd() const
{
    XenPtr<xen_sr_record> record = getRecord();
    return _session.getStore().getPbd((record->pbds && record->pbds->size) ? record->pbds->contents[0] : NULL);
}


bool StorageRepository::setName(const char* name, const char* desc)
{
    if (!xen_sr_set_name_label(_session, _handle, (char*)name))
    {
        emit(ERROR);
        return false;
    }

    if (!xen_sr_set_name_description(_session, _handle, (char*)desc))
    {
        emit(ERROR);
        return false;
    }

    return true;
}


bool StorageRepository::isCifs() const
{
    RefPtr<PhysicalBlockDevice> pbd = getPbd();
    if (pbd)
    {
        XenPtr<xen_pbd_record> pbdRecord = pbd->getRecord();
        return XenServer::match(pbdRecord->device_config, "type", "cifs") == 1;
    }
    return false;
}


bool StorageRepository::isTools() const
{
    XenPtr<xen_sr_record> record = getRecord();
    return XenServer::match(record->other_config, "xenserver_tools_sr", "true") == 1;
}


bool StorageRepository::isDefault()
{
    return XenServer::isDefaultSr(_session, _handle);
}


void StorageRepository::setStatus()
{
    StringBuffer buffer;

    switch (_subType)
    {
    case DEV:
    case ISO:
    {
        RefPtr<PhysicalBlockDevice> pbd = getPbd();
        if (pbd)
        {
            XenPtr<xen_pbd_record> pbdRecord = pbd->getRecord();
            if (pbdRecord)
            {
                if (pbdRecord->currently_attached)
                {
                    buffer += gettext("Attached");
                }
                else
                {
                    buffer += gettext("Detached");
                }
            }
        }
        break;
    }

    default:
    {
        XenPtr<xen_sr_record> record = getRecord();
        FormatSize(buffer, record->physical_size);
        if (record->physical_size)
        {
            buffer.appendFormat(gettext(" (%ld%% used)"),
                                (record->physical_utilisation * 100) / record->physical_size);
        }
        if (isDefault())
        {
            buffer += gettext(" [default]");
        }
        break;
    }
    }

    setDisplayStatus(buffer);
}


bool StorageRepository::remove()
{
    XenPtr<xen_sr_record> record = getRecord();
    if (record->pbds)
    {
        for (size_t i = 0; i < record->pbds->size; i++)
        {
            RefPtr<PhysicalBlockDevice> pbd = _session.getStore().getPbd(record->pbds->contents[i]);
            if (!pbd)
            {
                continue;
            }
            XenPtr<xen_pbd_record> record = pbd->getRecord();
            const char* value = XenServer::find(record->device_config, "cifspassword_secret");
            if (value)
            {
                XenRef<xen_secret, xen_secret_free_t> secret;
                if (!xen_secret_get_by_uuid(_session, &secret, const_cast<char*>(value)))
                {
                    pbd->emit(ERROR);
                    return false;
                }
                else if (!xen_secret_destroy(_session, secret))
                {
                    pbd->emit(ERROR);
                    return false;
                }
            }
            if (!xen_pbd_unplug(_session, pbd->getHandle()))
            {
                pbd->emit(ERROR);
                return false;
            }
            else if (!xen_pbd_destroy(_session, pbd->getHandle()))
            {
                pbd->emit(ERROR);
                return false;
            }
        }
    }

    if (!xen_sr_forget(_session, _handle))
    {
        emit(ERROR);
        return false;
    }

    return true;
}
