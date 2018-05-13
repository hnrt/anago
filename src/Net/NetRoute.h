// Copyright (C) 2012-2018 Hideaki Narita


#ifndef NETROUTE_H
#define NETROUTE_H


#include <netinet/in.h>
#include <glibmm.h>


namespace hnrt
{
    struct NetRoute
    {
        struct Record
        {
            Glib::ustring name;
            unsigned int destination;
            unsigned int gateway;
            unsigned int flags;
            unsigned int refcnt;
            unsigned int use;
            unsigned int metric;
            unsigned int mask;
            unsigned int mtu;
            unsigned int window;
            unsigned int irtt;
            Record();
            Record(const Record&);
            ~Record();
            bool parse(const char*);
        };

        NetRoute();
        NetRoute(const NetRoute&);
        ~NetRoute();
        int getSize() const { return _size; }
        const Record& operator [](int i) const { return *(_base + i); }
        in_addr_t getBroadcastAddress(in_addr_t);
        NetRoute& operator =(const NetRoute&);

    protected:

        Record* _base;
        int _size;
        int _capacity;
    };
}


#endif //!NETROUTE_H
