// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_SESSION_H
#define HNRT_SESSION_H


#include "Model/ConnectSpec.h"
#include "XenObject.h"


namespace hnrt
{
    class Session
        : public XenObject
    {
    public:

        Session(const ConnectSpec& cs);
        Session();
        virtual ~Session();
        const ConnectSpec& getConnectSpec() const { return _connectSpec; }
        ConnectSpec& getConnectSpec() { return _connectSpec; }
        xen_session* ptr() const { return _ptr; }
        operator xen_session*() const { return _ptr; }
        xen_session* operator ->() const { return _ptr; }
        bool isConnected() const;
        operator bool() const { return isConnected(); }
        bool connect();
        bool connect(const Session& session);
        bool disconnect();
        bool succeeded() const;
        bool failed() const;
        void clearError();
        bool hasError() const;
        bool hasError(const char*) const;
        const XenObjectStore& getStore() const { return *_objectStore; }
        XenObjectStore& getStore() { return *_objectStore; }
        bool operator ==(const Session&) const;
        bool operator !=(const Session&) const;
        void setMonitoring(bool value) { _monitoring = value; }
        const char* url() const { return _url.c_str(); }

        class Lock
        {
        public:

            Lock(Session& session)
                : _mutex(session._mutex)
            {
                _mutex.lock();
            }

            ~Lock()
            {
                _mutex.unlock();
            }

        private:

            Glib::Mutex& _mutex;
        };

    protected:

        enum State
        {
            NONE,
            PRIMARY_PENDING,
            PRIMARY,
            SECONDARY_PENDING,
            SECONDARY,
        };

        Session(const Session&);
        void operator =(const Session&);

        volatile int _state;
        ConnectSpec _connectSpec;
        Glib::ustring _url;
        xen_session* _ptr;
        RefPtr<XenObjectStore> _objectStore;
        volatile bool _monitoring;
    };
}


#endif //!HNRT_SESSION_H
