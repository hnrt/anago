// Copyright (C) 2012-2017 Hideaki Narita


#ifndef WAKEONLAN_H
#define WAKEONLAN_H


namespace hnrt
{
    struct MacAddress;

    struct WakeOnLan
    {
        unsigned char packet[102];

        WakeOnLan(const MacAddress&);
        int send(const char*);

    private:

        WakeOnLan(const WakeOnLan&);
        void operator =(const WakeOnLan&);
    };
}


#endif //!WAKEONLAN_H
