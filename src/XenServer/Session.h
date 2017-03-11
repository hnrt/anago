// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_SESSION_H
#define HNRT_SESSION_H


#include <glibmm.h>
#include "Model/ConnectSpec.h"
#include "Api.h"
#include "XenObject.h"


namespace hnrt
{
    class XenObjectStore;

    class Session
        : public XenObject
    {
    public:

        enum State
        {
            NONE,
            PRIMARY_PENDING,
            PRIMARY,
            SECONDARY_PENDING,
            SECONDARY,
        };

        Session(const ConnectSpec& cs);
        Session();
        virtual ~Session();
        xen_session* ptr() const { return _ptr; }
        operator xen_session*() const { return _ptr; }
        xen_session* operator ->() const { return _ptr; }
        bool isConnected() const;
        operator bool() const { return isConnected(); }
        const ConnectSpec& getConnectSpec() const { return _connectSpec; }
        ConnectSpec& getConnectSpec() { return _connectSpec; }
        const char* url() const { return _url.c_str(); }
        bool connect();
        bool connect(const Session& session);
        bool disconnect();
        bool succeeded();
        bool failed();
        void clearError();
        bool hasError();
        bool hasError(const char* error);
        const XenObjectStore& getStore() const { return *_objectStore; }
        XenObjectStore& getStore() { return *_objectStore; }
        bool operator ==(const Session&) const;
        void setMonitoring(bool value) { _monitoring = value; }

    protected:

        Session(const Session&);
        void operator =(const Session&);

        ConnectSpec _connectSpec;
        Glib::ustring _url;
        xen_session* _ptr;
        State _state;
        RefPtr<XenObjectStore> _objectStore;
        bool _monitoring;
    };
}


#endif //!ANAGO_SESSION_H
