// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_HOST_H
#define HNRT_HOST_H


#include <list>
#include "XenObject.h"


namespace hnrt
{
    struct PatchRecord;

    class Host
        : public XenObject
    {
    public:

        static RefPtr<Host> create(const ConnectSpec&);

        virtual ~Host();
        virtual int setBusy(bool = true);
        XenPtr<xen_host_record> getRecord() const;
        void setRecord(const XenPtr<xen_host_record>&);
        XenPtr<xen_host_metrics_record> getMetricsRecord() const;
        void setMetricsRecord(const XenPtr<xen_host_metrics_record>&);
        void onConnectPending();
        bool onConnected();
        void onConnectFailed();
        void onDisconnectPending();
        void onDisconnected();
        void onDisconnectedByPeer();
        void notifyDisconnection();
        bool shutdown();
        bool reboot();
        bool setName(const char* label, const char* description);
        void initPatchList();
        void updatePatchList();
        int getPatchList(std::list<RefPtr<PatchRecord> >&) const;
        RefPtr<PatchRecord> getPatchRecord(const Glib::ustring&) const;
        bool applyPatch(const Glib::ustring&);
        bool cleanPatch(const Glib::ustring&);

    protected:

        enum State
        {
            STATE_NONE,
            STATE_CONNECT_PENDING,
            STATE_CONNECTED,
            STATE_CONNECT_FAILED,
            STATE_DISCONNECT_PENDING,
            STATE_DISCONNECTED,
            STATE_DISCONNECTED_BY_PEER,
            STATE_DISCONNECT_FAILED,
            STATE_SHUTDOWN_PENDING,
            STATE_SHUTDOWN,
            STATE_SHUTDOWN_FAILED,
            STATE_REBOOT_PENDING,
            STATE_REBOOTED,
            STATE_REBOOT_FAILED,
        };

        Host(Session& session);
        Host(const Host&);
        void operator =(const Host&);

        State _state;
        XenPtr<xen_host_record> _record;
        XenPtr<xen_host_metrics_record> _metricsRecord;
        std::list<RefPtr<PatchRecord> > _patchList;
    };
}


#endif //!HNRT_HOST_H
