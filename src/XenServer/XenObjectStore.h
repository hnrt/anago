// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_XENOBJECTSTORE_H
#define HNRT_XENOBJECTSTORE_H


#include <list>
#include <map>
#include <glibmm.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"
#include "XenServer/Api.h"
#include "XenServer/XenObject.h"


namespace hnrt
{
    class Host;
    //class Network;
    //class PerformanceMonitor;
    //class PhysicalBlockDevice;
    //class PhysicalInterface;
    //class StorageRepository;
    //class VirtualBlockDevice;
    //class VirtualDiskImage;
    //class VirtualInterface;
    //class VirtualMachine;
    class XenTask;

    class XenObjectStore : public RefObj
    {
    public:

        typedef std::map<Glib::ustring, RefPtr<XenObject> > HandleObjectMap;
        typedef std::pair<Glib::ustring, RefPtr<XenObject> > HandleObjectEntry;
        typedef std::map<XenObject::Type, HandleObjectMap*> TypeMapMap;
        typedef std::pair<XenObject::Type, HandleObjectMap*> TypeMapEntry;

        XenObjectStore();
        virtual ~XenObjectStore();
        void clear();
        RefPtr<XenObject> get(const Glib::ustring& key, XenObject::Type type = XenObject::ANY);
        RefPtr<XenObject> get(const char* key, XenObject::Type type = XenObject::ANY);
        void remove(const RefPtr<XenObject>& object);
        void remove(const Glib::ustring& refid, XenObject::Type type = XenObject::ANY);
        void remove(const char* refid, XenObject::Type type = XenObject::ANY) { return remove(Glib::ustring(refid), type); }
        template<typename T> void add(RefPtr<T>& object) { addObject(RefPtr<XenObject>::castStatic(object)); }
        RefPtr<Host> getHost();
        void setHost(const RefPtr<Host>&);
        void removeHost();
        //RefPtr<PerformanceMonitor> getPerformanceMonitor() const;
        //void setPerformanceMonitor(const RefPtr<PerformanceMonitor>&);
        //RefPtr<Network> getNw(const Glib::ustring& key) const;
        //RefPtr<Network> getNw(const char* key) const;
        //RefPtr<Network> getNw(const xen_network_record_opt* opt) const;
        //RefPtr<PhysicalBlockDevice> getPbd(const Glib::ustring& key) const;
        //RefPtr<PhysicalBlockDevice> getPbd(const char* key) const;
        //RefPtr<PhysicalBlockDevice> getPbd(const xen_pbd_record_opt* opt) const;
        //RefPtr<PhysicalInterface> getPif(const Glib::ustring& key) const;
        //RefPtr<PhysicalInterface> getPif(const char* key) const;
        //RefPtr<PhysicalInterface> getPif(const xen_pif_record_opt* opt) const;
        //RefPtr<StorageRepository> getSr(const Glib::ustring& key) const;
        //RefPtr<StorageRepository> getSr(const char* key) const;
        //RefPtr<StorageRepository> getSr(const xen_sr_record_opt* opt) const;
        //RefPtr<VirtualBlockDevice> getVbd(const Glib::ustring& key) const;
        //RefPtr<VirtualBlockDevice> getVbd(const char* key) const;
        //RefPtr<VirtualBlockDevice> getVbd(const xen_vbd_record_opt* opt) const;
        //RefPtr<VirtualDiskImage> getVdi(const Glib::ustring& key) const;
        //RefPtr<VirtualDiskImage> getVdi(const char* key) const;
        //RefPtr<VirtualDiskImage> getVdi(const xen_vdi_record_opt* opt) const;
        //RefPtr<VirtualInterface> getVif(const Glib::ustring& key) const;
        //RefPtr<VirtualInterface> getVif(const char* key) const;
        //RefPtr<VirtualInterface> getVif(const xen_vif_record_opt* opt) const;
        //RefPtr<VirtualMachine> getVm(const Glib::ustring& key) const;
        //RefPtr<VirtualMachine> getVm(const char* key) const;
        //RefPtr<VirtualMachine> getVm(const xen_vm_record_opt* opt) const;
        //RefPtr<VirtualMachine> getVmByMetrics(const xen_vm_metrics) const;
        //RefPtr<VirtualMachine> getVmByGuestMetrics(const xen_vm_guest_metrics) const;
        //RefPtr<VirtualMachine> getVmByImportTask(const Glib::ustring& key) const;
        //RefPtr<Task> getTask(const Glib::ustring& key) const;
        //RefPtr<Task> getTask(const char* key) const;
        //int getList(std::list<RefPtr<Network> >& list) const;
        //int getList(std::list<RefPtr<PhysicalInterface> >& list) const;
        //int getList(std::list<RefPtr<StorageRepository> >& list) const;
        //int getList(std::list<RefPtr<VirtualInterface> >& list) const;
        //int getList(std::list<RefPtr<VirtualMachine> >& list) const;
        //Glib::ustring getSrCandidate(int64_t hint, const Glib::ustring& defaultSr) const;
        //void debuginfo();

    protected:

        XenObjectStore(const XenObjectStore&);
        void operator =(const XenObjectStore&);
        void addObject(RefPtr<XenObject>);
        template<typename T> int getList(std::list<RefPtr<T> >& list, XenObject::Type type) const;

        Glib::RecMutex _mutex;
        RefPtr<Host> _host;
        //RefPtr<PerformanceMonitor> _performanceMonitor;
        TypeMapMap _typeMapMap;
    };
}


#endif //!HNRT_XENOBJECTSTORE_H
