// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_XENOBJECT_H
#define HNRT_XENOBJECT_H


#include <glibmm.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class Session;

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
            DESTROYED = 511,
            NOTIFICATION_MIN = CREATED,
            NOTIFICATION_MAX = DESTROYED,
        };

        virtual ~XenObject();
        Type getType() const { return _type; }
        const Session& getSession() const;
        Session& getSession();
        void* getXenRef() const { return const_cast<void*>(reinterpret_cast<const void*>(_refid.c_str())); }
        const Glib::ustring& getREFID() const { return _refid; }
        const Glib::ustring& getUUID() const { return _uuid; }
        void lock();
        void unlock();
        Glib::ustring getName();
        virtual void setName(const char*);
        Glib::ustring getDisplayStatus();
        virtual void setDisplayStatus(const char*);
        bool isBusy() const { return _busyCount > 0; }
        virtual int setBusy(bool = true);
        void emit(Notification);

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

        XenObject(Type type, Session& session, const char* refid, const char* uuid = 0, const char* name = 0);
        XenObject(const XenObject&);
        void operator =(const XenObject&);

        Type _type;
        Session& _session;
        Glib::ustring _refid;
        Glib::RecMutex _mutex;
        Glib::ustring _uuid;
        Glib::ustring _name;
        Glib::ustring _displayStatus;
        volatile int _busyCount;
    };
}


#endif //!HNRT_XENOBJECT_H
