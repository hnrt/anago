// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_XENEVENTMONITOR_H
#define HNRT_XENEVENTMONITOR_H


namespace hnrt
{
    class Session;

    class XenEventMonitor
    {
    public:

        XenEventMonitor();
        ~XenEventMonitor();
        void run(Session&);

    private:

        struct Record
        {
            int64_t id;
            time_t timestamp;
            int type;
            xen_event_operation operation;
            char *ref;

            Record(int, const xen_event_record*);
            bool process(XenEventMonitor&, Session& primary, Session& secondary);
        };

        XenEventMonitor(const XenEventMonitor&);
        void operator =(const XenEventMonitor&);
        void disconnect() { _connected = false; }

        bool _connected;
    };
}


#endif //!HNRT_XENEVENTMONITOR_H
