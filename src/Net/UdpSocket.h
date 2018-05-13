// Copyright (C) 2012-2018 Hideaki Narita


#ifndef UDPSOCKET_H
#define UDPSOCKET_H


namespace hnrt
{
    struct UdpSocket
    {
        int fd;

        UdpSocket();
        ~UdpSocket();
        operator int() const { return fd; }
        int setBroadcast(bool value = true);

    private:

        UdpSocket(const UdpSocket&);
        void operator =(const UdpSocket&);
    };
}


#endif //!UDPSOCKET_H
