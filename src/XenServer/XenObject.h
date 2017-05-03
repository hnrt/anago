// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_XENOBJECT_H
#define HNRT_XENOBJECT_H


#include <glibmm.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"
#include "XenServer/Api.h"
#include "XenServer/XenPtr.h"
#include "XenServer/XenRef.h"


namespace hnrt
{
    class Host;
    class Network;
    class PhysicalBlockDevice;
    class PhysicalInterface;
    class Session;
    class StorageRepository;
    class VirtualBlockDevice;
    class VirtualDiskImage;
    class VirtualInterface;
    class VirtualMachine;
    class XenObjectStore;
    class XenTask;
    struct ConnectSpec;

    class XenObject
        : public RefObj
    {
    public:

        enum Type
        {
            NONE,
            ANY,
            HOST,
            HOST_METRICS,
            NETWORK,
            PBD,
            PIF,
            POOL,
            POOL_PATCH,
            SESSION,
            SR,
            TASK,
            VBD,
            VDI,
            VIF,
            VM,
            VM_METRICS,
            VM_GUEST_METRICS,
            VM_EXPORTER,
            VM_IMPORTER,
            VM_VERIFIER,
        };

        enum Notification
        {
            CREATED = 257,
            BUSY_SET,
            BUSY_RESET,
            NAME_UPDATED,
            STATUS_UPDATED,
            SESSION_UPDATED,
            CONNECTED,
            CONNECT_FAILED,
            DISCONNECTED,
            POWER_STATE_UPDATED,
            RECORD_UPDATED,
            SNAPSHOT_UPDATED,
            TASK_ON_SUCCESS,
            TASK_ON_FAILURE,
            TASK_ON_CANCELLED,
            TASK_IN_PROGRESS,
            PERFORMANCE_STATS_UPDATED,
            EXPORT_PENDING,
            EXPORTING,
            EXPORTED,
            EXPORT_FAILED,
            EXPORT_CANCELED,
            IMPORT_PENDING,
            IMPORTING,
            IMPORTED,
            IMPORT_FAILED,
            IMPORT_CANCELED,
            VERIFY_PENDING,
            VERIFYING,
            VERIFIED,
            VERIFY_FAILED,
            VERIFY_CANCELED,
            ERROR,
            DESTROYED = 511,
            NOTIFICATION_MIN = CREATED,
            NOTIFICATION_MAX = DESTROYED,
        };

        virtual ~XenObject();
        Type getType() const { return _type; }
        const Session& getSession() const { return _session; }
        Session& getSession() { return _session; }
        void* getHandle() const { return _handle; }
        Glib::ustring getREFID() const { return _refid; }
        Glib::ustring getUUID() const { return _uuid; }
        Glib::ustring getName() const;
        virtual void setName(const char*);
        virtual Glib::ustring getDisplayStatus() const;
        virtual void setDisplayStatus(const char*);
        virtual bool isBusy() const { return _busyCount > 0; }
        virtual int setBusy(bool = true);
        virtual void emit(Notification);

        class Busy
        {
        public:
            Busy(XenObject* pThis) : _pThis(pThis) { _pThis->setBusy(true); }
            ~Busy() { _pThis->setBusy(false); }
        private:
            Busy(const Busy&);
            void operator =(const Busy&);
            XenObject* _pThis;
        };

    protected:

        XenObject(Type, Session&, void* handle, const char* uuid, const char* name);
        XenObject(const XenObject&);
        void operator =(const XenObject&);

        Type _type;
        Session& _session;
        void* _handle;
        Glib::ustring _refid;
        Glib::ustring _uuid;
        Glib::Mutex _mutex;
        Glib::ustring _name;
        Glib::ustring _displayStatus;
        volatile int _busyCount;
    };

    const char* GetXenObjectTypeText(const XenObject&);
    const char* GetNotificationText(int);
}


#endif //!HNRT_XENOBJECT_H
