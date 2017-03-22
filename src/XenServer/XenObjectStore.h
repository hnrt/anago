// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_XENOBJECTSTORE_H
#define HNRT_XENOBJECTSTORE_H


#include <glibmm.h>
#include <list>
#include <map>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"
#include "XenServer/Api.h"
#include "XenServer/XenObject.h"


namespace hnrt
{
    class PerformanceMonitor;

    class XenObjectStore
        : public RefObj
    {
    public:

        typedef std::map<Glib::ustring, RefPtr<XenObject> > HandleObjectMap;
        typedef std::pair<Glib::ustring, RefPtr<XenObject> > HandleObjectEntry;
        typedef std::map<XenObject::Type, HandleObjectMap*> TypeMapMap;
        typedef std::pair<XenObject::Type, HandleObjectMap*> TypeMapEntry;

        XenObjectStore();
        virtual ~XenObjectStore();
        RefPtr<Host> getHost() const;
        void setHost(const RefPtr<Host>&);
        void removeHost();
        void clear();
        RefPtr<XenObject> get(const Glib::ustring&, XenObject::Type = XenObject::ANY) const;
        RefPtr<XenObject> get(const char*, XenObject::Type = XenObject::ANY) const;
        void remove(const RefPtr<XenObject>&);
        void remove(const Glib::ustring&, XenObject::Type = XenObject::ANY);
        void remove(const char* refid, XenObject::Type type = XenObject::ANY) { return remove(Glib::ustring(refid), type); }
        template<typename T> void add(RefPtr<T>& object) { addObject(RefPtr<XenObject>::castStatic(object)); }
        RefPtr<PerformanceMonitor> getPerformanceMonitor() const;
        void setPerformanceMonitor(const RefPtr<PerformanceMonitor>&);
        void removePerformanceMonitor();
        RefPtr<Network> getNw(const Glib::ustring&) const;
        RefPtr<Network> getNw(const char*) const;
        RefPtr<Network> getNw(const xen_network_record_opt*) const;
        RefPtr<PhysicalBlockDevice> getPbd(const Glib::ustring&) const;
        RefPtr<PhysicalBlockDevice> getPbd(const char*) const;
        RefPtr<PhysicalBlockDevice> getPbd(const xen_pbd_record_opt*) const;
        RefPtr<PhysicalInterface> getPif(const Glib::ustring&) const;
        RefPtr<PhysicalInterface> getPif(const char*) const;
        RefPtr<PhysicalInterface> getPif(const xen_pif_record_opt*) const;
        RefPtr<StorageRepository> getSr(const Glib::ustring&) const;
        RefPtr<StorageRepository> getSr(const char*) const;
        RefPtr<StorageRepository> getSr(const xen_sr_record_opt*) const;
        RefPtr<XenTask> getTask(const Glib::ustring&) const;
        RefPtr<XenTask> getTask(const char*) const;
        RefPtr<VirtualBlockDevice> getVbd(const Glib::ustring&) const;
        RefPtr<VirtualBlockDevice> getVbd(const char*) const;
        RefPtr<VirtualBlockDevice> getVbd(const xen_vbd_record_opt*) const;
        RefPtr<VirtualDiskImage> getVdi(const Glib::ustring&) const;
        RefPtr<VirtualDiskImage> getVdi(const char*) const;
        RefPtr<VirtualDiskImage> getVdi(const xen_vdi_record_opt*) const;
        RefPtr<VirtualInterface> getVif(const Glib::ustring&) const;
        RefPtr<VirtualInterface> getVif(const char*) const;
        RefPtr<VirtualInterface> getVif(const xen_vif_record_opt*) const;
        RefPtr<VirtualMachine> getVm(const Glib::ustring&) const;
        RefPtr<VirtualMachine> getVm(const char*) const;
        RefPtr<VirtualMachine> getVm(const xen_vm_record_opt*) const;
        RefPtr<VirtualMachine> getVmByMetrics(const xen_vm_metrics) const;
        RefPtr<VirtualMachine> getVmByGuestMetrics(const xen_vm_guest_metrics) const;
        RefPtr<VirtualMachine> getVmByImportTask(const Glib::ustring& key) const;
        int getList(std::list<RefPtr<Network> >&) const;
        int getList(std::list<RefPtr<PhysicalInterface> >&) const;
        int getList(std::list<RefPtr<StorageRepository> >&) const;
        int getList(std::list<RefPtr<VirtualInterface> >&) const;
        int getList(std::list<RefPtr<VirtualMachine> >&) const;
        Glib::ustring getSrCandidate(int64_t hint, const Glib::ustring& defaultSr) const;
        //void debuginfo();

    protected:

        XenObjectStore(const XenObjectStore&);
        void operator =(const XenObjectStore&);
        void addObject(RefPtr<XenObject>);
        template<typename T> RefPtr<XenObject> getByOpt(T, XenObject::Type) const;
        template<typename T> int getListByType(std::list<RefPtr<T> >&, XenObject::Type) const;

        Glib::RecMutex _mutex;
        RefPtr<Host> _host;
        RefPtr<PerformanceMonitor> _performanceMonitor;
        TypeMapMap _typeMapMap;
    };
}


#endif //!HNRT_XENOBJECTSTORE_H
