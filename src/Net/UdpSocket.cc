// Copyright (C) 2012-2017 Hideaki Narita


#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "UdpSocket.h"


using namespace hnrt;


UdpSocket::UdpSocket()
    : fd(socket(AF_INET, SOCK_DGRAM, 0))
{
}


UdpSocket::~UdpSocket()
{
    if (fd >= 0)
    {
        close(fd);
    }
}


int UdpSocket::setBroadcast(bool value)
{
    int broadcast = value ? 1 : 0;
    return setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
}
