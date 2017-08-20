// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PINGAGENTIMPL_H
#define HNRT_PINGAGENTIMPL_H


#include <time.h>
#include <glibmm.h>
#include <list>
#include "PingAgent.h"


namespace hnrt
{
    class PingAgentImpl
        : public PingAgent
    {
    public:

        PingAgentImpl();
        virtual ~PingAgentImpl();
        virtual void open();
        virtual void close();
        virtual void clear();
        virtual void add(const char*);
        virtual void remove(const char*);
        virtual State get(const char*);

    private:

        struct Record
        {
            Glib::ustring hostname;
            State state;
            time_t timestamp;
        };

        PingAgentImpl(const PingAgentImpl&);
        void operator =(const PingAgentImpl&);
        void append(const char*);
        void run();
        void check();
        void getHostnames(std::list<Glib::ustring>&);
        State check(const char*);
        void update(const Glib::ustring&, State);

        Glib::Thread* _thread;
        Glib::Mutex _mutexThread;
        Glib::Cond _cond;
        bool _stop;
        Glib::Mutex _mutexRecords;
        std::list<Record> _records;
        int _interval;
    };
}


#endif //!HNRT_PINGAGENTIMPL_H
