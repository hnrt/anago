// Copyright (C) 2012-2017 Hideaki Narita


#include <string.h>
#include <list>
#include "Controller/Controller.h"
//#include "Controller/PerformanceMonitor.h"
//#include "Network.h"
#include "PhysicalBlockDevice.h"
//#include "PhysicalInterface.h"
#include "StorageRepository.h"
#include "XenTask.h"
#include "VirtualBlockDevice.h"
#include "VirtualDiskImage.h"
//#include "VirtualInterface.h"
#include "VirtualMachine.h"
#include "Host.h"
#include "Macros.h"
#include "XenObjectStore.h"


using namespace hnrt;


XenObjectStore::XenObjectStore()
{
}


XenObjectStore::~XenObjectStore()
{
    clear();
}


RefPtr<Host> XenObjectStore::getHost()
{
    Glib::RecMutex::Lock k(_mutex);
    return _host;
}


void XenObjectStore::setHost(const RefPtr<Host>& host)
{
    RefPtr<Host> prev;
    RefPtr<Host> next;
    {
        Glib::RecMutex::Lock k(_mutex);
        prev = _host;
        next = _host = host;
    }
    if (prev)
    {
        prev->emit(XenObject::DESTROYED);
    }
    if (next)
    {
        next->emit(XenObject::CREATED);
    }
}


void XenObjectStore::removeHost()
{
    RefPtr<Host> prev;
    {
        Glib::RecMutex::Lock k(_mutex);
        prev = _host;
        _host = RefPtr<Host>();
    }
    if (prev)
    {
        prev->emit(XenObject::DESTROYED);
    }
}


void XenObjectStore::clear()
{
    std::list<RefPtr<XenObject> > a;
    {
        Glib::RecMutex::Lock k(_mutex);
        for (TypeMapMap::iterator iter2 = _typeMapMap.begin(); iter2 != _typeMapMap.end(); iter2++)
        {
            HandleObjectMap* map = iter2->second;
            for (HandleObjectMap::const_iterator iter = map->begin(); iter != map->end(); iter++)
            {
                RefPtr<XenObject> object = iter->second;
                a.push_back(object);
            }
            iter2->second = NULL;
            delete map;
        }
        _typeMapMap.clear();
    }
    for (std::list<RefPtr<XenObject> >::const_iterator iter = a.begin(); iter != a.end(); iter++)
    {
        RefPtr<XenObject> object = *iter;
        object->emit(XenObject::DESTROYED);
    }
    //setPerformanceMonitor(RefPtr<PerformanceMonitor>());
}


RefPtr<XenObject> XenObjectStore::get(const Glib::ustring& key, XenObject::Type type)
{
    if (!key.empty())
    {
        Glib::RecMutex::Lock lock(_mutex);
        if (type == XenObject::ANY)
        {
            for (TypeMapMap::const_iterator iter2 = _typeMapMap.begin(); iter2 != _typeMapMap.end(); iter2++)
            {
                HandleObjectMap& map = *iter2->second;
                if (IS_REF(key.c_str()))
                {
                    HandleObjectMap::const_iterator iter = map.find(key);
                    if (iter != map.end())
                    {
                        return RefPtr<XenObject>(iter->second);
                    }
                }
                else
                {
                    for (HandleObjectMap::const_iterator iter = map.begin(); iter != map.end(); iter++)
                    {
                        if (iter->second->getUUID() == key)
                        {
                            return RefPtr<XenObject>(iter->second);
                        }
                    }
                }
            }
        }
        else
        {
            TypeMapMap::const_iterator iter2 = _typeMapMap.find(type);
            if (iter2 != _typeMapMap.end())
            {
                HandleObjectMap& map = *iter2->second;
                if (IS_REF(key.c_str()))
                {
                    HandleObjectMap::const_iterator iter = map.find(key);
                    if (iter != map.end())
                    {
                        return RefPtr<XenObject>(iter->second);
                    }
                }
                else
                {
                    for (HandleObjectMap::const_iterator iter = map.begin(); iter != map.end(); iter++)
                    {
                        if (iter->second->getUUID() == key)
                        {
                            return RefPtr<XenObject>(iter->second);
                        }
                    }
                }
            }
        }
    }
    return RefPtr<XenObject>();
}


RefPtr<XenObject> XenObjectStore::get(const char* key, XenObject::Type type)
{
    return get(Glib::ustring(key), type);
}


void XenObjectStore::addObject(RefPtr<XenObject> object)
{
    if (!object)
    {
        return;
    }
    const Glib::ustring& refid = object->getREFID();
    if (refid.empty())
    {
        return;
    }
    RefPtr<XenObject> old;
    XenObject::Type type = object->getType();
    if (type == XenObject::HOST)
    {
        Glib::RecMutex::Lock k(_mutex);
        old = RefPtr<XenObject>::castStatic(_host);
        _host = RefPtr<Host>::castStatic(object);
    }
    else
    {
        Glib::RecMutex::Lock k(_mutex);
        TypeMapMap::const_iterator iter2 = _typeMapMap.find(type);
        if (iter2 != _typeMapMap.end())
        {
            HandleObjectMap* map = iter2->second;
            HandleObjectMap::iterator iter = map->find(refid);
            if (iter != map->end())
            {
                old = iter->second;
                if (old != object)
                {
                    iter->second = object;
                }
            }
            else
            {
                map->insert(HandleObjectEntry(refid, object));
            }
        }
        else
        {
            HandleObjectMap* map = new HandleObjectMap;
            _typeMapMap.insert(TypeMapEntry(type, map));
            map->insert(HandleObjectEntry(refid, object));
        }
    }
    if (object != old)
    {
        if (old)
        {
            old->emit(XenObject::DESTROYED);
        }
        object->emit(XenObject::CREATED);
    }
}


void XenObjectStore::remove(const RefPtr<XenObject>& object)
{
    if (!object)
    {
        return;
    }
    remove(object->getREFID(), object->getType());
}


void XenObjectStore::remove(const Glib::ustring& refid, XenObject::Type type)
{
    if (refid.empty())
    {
        return;
    }
    RefPtr<XenObject> object;
    {
        Glib::RecMutex::Lock lock(_mutex);
        if (type == XenObject::ANY)
        {
            for (TypeMapMap::const_iterator iter2 = _typeMapMap.begin(); iter2 != _typeMapMap.end(); iter2++)
            {
                HandleObjectMap& map = *iter2->second;
                HandleObjectMap::iterator iter = map.find(refid);
                if (iter != map.end())
                {
                    object = iter->second;
                    map.erase(iter);
                    break;
                }
            }
        }
        else
        {
            TypeMapMap::const_iterator iter2 = _typeMapMap.find(type);
            if (iter2 != _typeMapMap.end())
            {
                HandleObjectMap& map = *iter2->second;
                HandleObjectMap::iterator iter = map.find(refid);
                if (iter != map.end())
                {
                    object = iter->second;
                    map.erase(iter);
                }
            }
        }
    }
    if (object)
    {
        object->emit(XenObject::DESTROYED);
    }
}


#if 0
RefPtr<PerformanceMonitor> XenObjectStore::getPerformanceMonitor() const
{
    Glib::RecMutex::Lock lock(_mutex);
    return _performanceMonitor;
}


void XenObjectStore::setPerformanceMonitor(const RefPtr<PerformanceMonitor>& performanceMonitor)
{
    RefPtr<PerformanceMonitor> prev;
    RefPtr<PerformanceMonitor> next;
    {
        Glib::RecMutex::Lock lock(_mutex);
        prev = _performanceMonitor;
        next = _performanceMonitor = performanceMonitor;
    }
    if (prev)
    {
        Controller::instance().notify(RefPtr<RefObj>::castStatic(prev), NOTIF_PM_DESTROYED);
    }
    if (next)
    {
        Controller::instance().notify(RefPtr<RefObj>::castStatic(next), NOTIF_PM_CREATED);
    }
}
#endif

template<typename T> RefPtr<XenObject> XenObjectStore::getByOpt(T* opt, XenObject::Type type)
{
    return opt ? get(opt->is_record ? opt->u.record->uuid : reinterpret_cast<const char*>(opt->u.handle), type) : RefPtr<XenObject>();
}

#if 0
RefPtr<Network> XenObjectStore::getNw(const Glib::ustring& key) const
{
    return RefPtr<Network>::castStatic(get(key, XenObject::NETWORK));
}


RefPtr<Network> XenObjectStore::getNw(const char* key) const
{
    return getNw(Glib::ustring(key));
}


RefPtr<Network> XenObjectStore::getNw(const xen_network_record_opt* opt) const
{
    return RefPtr<Network>::castStatic(getByOpt(opt, XenObject::NETWORK));
}
#endif

RefPtr<PhysicalBlockDevice> XenObjectStore::getPbd(const Glib::ustring& key)
{
    return RefPtr<PhysicalBlockDevice>::castStatic(get(key, XenObject::PBD));
}


RefPtr<PhysicalBlockDevice> XenObjectStore::getPbd(const char* key)
{
    return getPbd(Glib::ustring(key));
}


RefPtr<PhysicalBlockDevice> XenObjectStore::getPbd(const xen_pbd_record_opt* opt)
{
    return RefPtr<PhysicalBlockDevice>::castStatic(getByOpt(opt, XenObject::PBD));
}

#if 0
RefPtr<PhysicalInterface> XenObjectStore::getPif(const Glib::ustring& key) const
{
    return RefPtr<PhysicalInterface>::castStatic(get(key, XenObject::PIF));
}


RefPtr<PhysicalInterface> XenObjectStore::getPif(const char* key) const
{
    return getPif(Glib::ustring(key));
}


RefPtr<PhysicalInterface> XenObjectStore::getPif(const xen_pif_record_opt* opt) const
{
    return RefPtr<PhysicalInterface>::castStatic(getByOpt(opt, XenObject::PIF));
}
#endif

RefPtr<StorageRepository> XenObjectStore::getSr(const Glib::ustring& key)
{
    return RefPtr<StorageRepository>::castStatic(get(key, XenObject::SR));
}


RefPtr<StorageRepository> XenObjectStore::getSr(const char* key)
{
    return getSr(Glib::ustring(key));
}


RefPtr<StorageRepository> XenObjectStore::getSr(const xen_sr_record_opt* opt)
{
    return RefPtr<StorageRepository>::castStatic(getByOpt(opt, XenObject::SR));
}


RefPtr<XenTask> XenObjectStore::getTask(const Glib::ustring& key)
{
    return RefPtr<XenTask>::castStatic(get(key, XenObject::TASK));
}


RefPtr<XenTask> XenObjectStore::getTask(const char* key)
{
    return getTask(Glib::ustring(key));
}


RefPtr<VirtualBlockDevice> XenObjectStore::getVbd(const Glib::ustring& key)
{
    return RefPtr<VirtualBlockDevice>::castStatic(get(key, XenObject::VBD));
}


RefPtr<VirtualBlockDevice> XenObjectStore::getVbd(const char* key)
{
    return getVbd(Glib::ustring(key));
}


RefPtr<VirtualBlockDevice> XenObjectStore::getVbd(const xen_vbd_record_opt* opt)
{
    return RefPtr<VirtualBlockDevice>::castStatic(getByOpt(opt, XenObject::VBD));
}


RefPtr<VirtualDiskImage> XenObjectStore::getVdi(const Glib::ustring& key)
{
    return RefPtr<VirtualDiskImage>::castStatic(get(key, XenObject::VDI));
}


RefPtr<VirtualDiskImage> XenObjectStore::getVdi(const char* key)
{
    return getVdi(Glib::ustring(key));
}


RefPtr<VirtualDiskImage> XenObjectStore::getVdi(const xen_vdi_record_opt* opt)
{
    return RefPtr<VirtualDiskImage>::castStatic(getByOpt(opt, XenObject::VDI));
}

#if 0
RefPtr<VirtualInterface> XenObjectStore::getVif(const Glib::ustring& key) const
{
    return RefPtr<VirtualInterface>::castStatic(get(key, XenObject::VIF));
}


RefPtr<VirtualInterface> XenObjectStore::getVif(const char* key) const
{
    return getVif(Glib::ustring(key));
}


RefPtr<VirtualInterface> XenObjectStore::getVif(const xen_vif_record_opt* opt) const
{
    return RefPtr<VirtualInterface>::castStatic(getByOpt(opt, XenObject::VIF));
}
#endif

RefPtr<VirtualMachine> XenObjectStore::getVm(const Glib::ustring& key)
{
    return RefPtr<VirtualMachine>::castStatic(get(key, XenObject::VM));
}


RefPtr<VirtualMachine> XenObjectStore::getVm(const char* key)
{
    return getVm(Glib::ustring(key));
}


RefPtr<VirtualMachine> XenObjectStore::getVm(const xen_vm_record_opt* opt)
{
    return RefPtr<VirtualMachine>::castStatic(getByOpt(opt, XenObject::VM));
}


RefPtr<VirtualMachine> XenObjectStore::getVmByMetrics(const xen_vm_metrics metrics)
{
    if (metrics)
    {
        Glib::RecMutex::Lock lock(_mutex);
        TypeMapMap::const_iterator iter2 = _typeMapMap.find(XenObject::VM);
        if (iter2 != _typeMapMap.end())
        {
            HandleObjectMap& map = *iter2->second;
            for (HandleObjectMap::const_iterator iter = map.begin(); iter != map.end(); iter++)
            {
                RefPtr<VirtualMachine> vm = RefPtr<VirtualMachine>::castStatic(iter->second);
                XenPtr<xen_vm_record> record = vm->getRecord();
                if (record->metrics &&
                    !record->metrics->is_record &&
                    !strcmp(reinterpret_cast<const char*>(record->metrics->u.handle), reinterpret_cast<const char*>(metrics)))
                {
                    RefPtr<XenObject> object(iter->second);
                    return RefPtr<VirtualMachine>::castStatic(object);
                }
            }
        }
    }
    return RefPtr<VirtualMachine>();
}


RefPtr<VirtualMachine> XenObjectStore::getVmByGuestMetrics(const xen_vm_guest_metrics guestMetrics)
{
    if (guestMetrics)
    {
        Glib::RecMutex::Lock lock(_mutex);
        TypeMapMap::const_iterator iter2 = _typeMapMap.find(XenObject::VM);
        if (iter2 != _typeMapMap.end())
        {
            HandleObjectMap& map = *iter2->second;
            for (HandleObjectMap::const_iterator iter = map.begin(); iter != map.end(); iter++)
            {
                RefPtr<VirtualMachine> vm = RefPtr<VirtualMachine>::castStatic(iter->second);
                XenPtr<xen_vm_record> record = vm->getRecord();
                if (record->guest_metrics &&
                    !record->guest_metrics->is_record &&
                    !strcmp(reinterpret_cast<const char*>(record->guest_metrics->u.handle), reinterpret_cast<const char*>(guestMetrics)))
                {
                    RefPtr<XenObject> object(iter->second);
                    return RefPtr<VirtualMachine>::castStatic(object);
                }
            }
        }
    }
    return RefPtr<VirtualMachine>();
}


RefPtr<VirtualMachine> XenObjectStore::getVmByImportTask(const Glib::ustring& key)
{
    Glib::RecMutex::Lock lock(_mutex);
    TypeMapMap::const_iterator iter2 = _typeMapMap.find(XenObject::VM);
    if (iter2 != _typeMapMap.end())
    {
        HandleObjectMap& map = *iter2->second;
        for (HandleObjectMap::const_iterator iter = map.begin(); iter != map.end(); iter++)
        {
            RefPtr<VirtualMachine> vm = RefPtr<VirtualMachine>::castStatic(iter->second);
            XenPtr<xen_vm_record> record = vm->getRecord();
            int rc = XenServer::match(record->other_config, "import_task", key.c_str());
            if (rc == 1)
            {
                return vm;
            }
        }
    }
    return RefPtr<VirtualMachine>();
}


template<typename T> int XenObjectStore::getList(std::list<RefPtr<T> >& list, XenObject::Type type)
{
    Glib::RecMutex::Lock lock(_mutex);
    int count = 0;
    TypeMapMap::const_iterator iter2 = _typeMapMap.find(type);
    if (iter2 != _typeMapMap.end())
    {
        HandleObjectMap& map = *iter2->second;
        for (HandleObjectMap::iterator iter = map.begin(); iter != map.end(); iter++)
        {
            list.push_back(RefPtr<T>::castStatic(iter->second));
            count++;
        }
    }
    return count;
}

#if 0
int XenObjectStore::getList(std::list<RefPtr<Network> >& list)
{
    return getList(list, XenObject::NETWORK);
}


int XenObjectStore::getList(std::list<RefPtr<PhysicalInterface> >& list)
{
    return getList(list, XenObject::PIF);
}
#endif

int XenObjectStore::getList(std::list<RefPtr<StorageRepository> >& list)
{
    return getList(list, XenObject::SR);
}

#if 0
int XenObjectStore::getList(std::list<RefPtr<VirtualInterface> >& list)
{
    return getList(list, XenObject::VIF);
}
#endif

int XenObjectStore::getList(std::list<RefPtr<VirtualMachine> >& list)
{
    return getList(list, XenObject::VM);
}


Glib::ustring XenObjectStore::getSrCandidate(int64_t hint, const Glib::ustring& defaultSr)
{
    Glib::ustring srREFID = defaultSr;
    RefPtr<StorageRepository> sr = getSr(srREFID);
    if (sr)
    {
        XenPtr<xen_sr_record> record = sr->getRecord();
        if (record)
        {
            int64_t space = record->physical_size - record->physical_utilisation;
            if (hint < space)
            {
                return srREFID;
            }
        }
    }
    Glib::RecMutex::Lock lock(_mutex);
    TypeMapMap::const_iterator iter2 = _typeMapMap.find(XenObject::SR);
    if (iter2 != _typeMapMap.end())
    {
        HandleObjectMap& map = *iter2->second;
        int64_t spaceMax = 0;
        for (HandleObjectMap::iterator iter = map.begin(); iter != map.end(); iter++)
        {
            sr = RefPtr<StorageRepository>::castStatic(iter->second);
            if (sr->getSubType() != StorageRepository::USR)
            {
                continue;
            }
            XenPtr<xen_sr_record> record = sr->getRecord();
            int64_t space = record->physical_size - record->physical_utilisation;
            if (spaceMax < space)
            {
                spaceMax = space;
                srREFID = sr->getREFID();
            }
        }
    }
    return srREFID;
}

#if 0
#include <stdio.h>
void XenObjectStore::debuginfo()
{
    Glib::RecMutex::Lock lock(_mutex);
    for (TypeMapMap::const_iterator iter2 = _typeMapMap.begin(); iter2 != _typeMapMap.end(); iter2++)
    {
        HandleObjectMap& map = *iter2->second;
        for (HandleObjectMap::const_iterator iter = map.begin(); iter != map.end(); iter++)
        {
            printf("#%s: %s\n", XenObjectTypeMap::toString(iter2->first), iter->first.str());
        }
    }
}
#endif
