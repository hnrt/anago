// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_XENEVENTMONITOR_H
#define HNRT_XENEVENTMONITOR_H


namespace hnrt
{
    class Session;

    class XenEventMonitor
    {
    public:

        XenEventMonitor(Session&);
        ~XenEventMonitor();
        Session& getSession() const { return _session; }
        void run();
        void disconnect() { _connected = false; }

    private:

        struct Record
        {
            int64_t id;
            time_t timestamp;
            int type;
            xen_event_operation operation;
            char *ref;

            Record(int, const xen_event_record*);
            bool process(Session& session, XenEventMonitor*);
        };

        XenEventMonitor(const XenEventMonitor&);
        void operator =(const XenEventMonitor&);

        Session& _session;
        bool _connected;
    };
}


#endif //!HNRT_XENEVENTMONITOR_H
