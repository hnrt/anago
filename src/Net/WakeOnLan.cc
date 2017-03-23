// Copyright (C) 2012-2017 Hideaki Narita


#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "HostEntry.h"
#include "MacAddress.h"
#include "NetRoute.h"
#include "UdpSocket.h"
#include "WakeOnLan.h"


using namespace hnrt;


WakeOnLan::WakeOnLan(const MacAddress& mac)
{
    unsigned char* cur = packet;
    memset(cur, 0xff, 6);
    cur += 6;
    for (int i = 0; i < 16; i++)
    {
        memcpy(cur, mac.value, 6);
        cur += 6;
    }
}


int WakeOnLan::send(const char* name)
{
    UdpSocket sock;
    if (sock < 0)
    {
        return errno;
    }

    if (sock.setBroadcast() < 0)
    {
        return errno;
    }

    int success = 0;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);

    HostEntry ent(name);
    if (ent.isValid())
    {
        NetRoute netRoute;
        for (HostEntry::Iter iter = ent.begin(); iter != ent.end(); iter++)
        {
            in_addr_t a = *iter;
            in_addr_t b = netRoute.getBroadcastAddress(a);
            if (b)
            {
                addr.sin_addr.s_addr = b;
                ssize_t sent = sendto(sock, packet, sizeof(packet), 0, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
                if (sent == sizeof(packet))
                {
                    success++;
                }
                else
                {
                    return errno;
                }
                break;
            }
        }
    }

    if (success)
    {
        return 0;
    }
    else
    {
        addr.sin_addr.s_addr = inet_addr("255.255.255.255");
        ssize_t sent = sendto(sock, packet, sizeof(packet), 0, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
        if (sent == sizeof(packet))
        {
            return 0;
        }
        else
        {
            return errno;
        }
    }
}
