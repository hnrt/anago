// Copyright (C) 2012-2018 Hideaki Narita


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AddressResolution.h"
#include "HostEntry.h"
#include "MacAddress.h"


using namespace hnrt;


MacAddress::MacAddress()
{
    memset(value, 0, 6);
}


MacAddress::MacAddress(const MacAddress& src)
{
    memcpy(value, src.value, 6);
}


bool MacAddress::isNull() const
{
    return
        value[0] == 0 &&
        value[1] == 0 &&
        value[2] == 0 &&
        value[3] == 0 &&
        value[4] == 0 &&
        value[5] == 0;
}


Glib::ustring MacAddress::toString() const
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
             value[0], value[1], value[2], value[3], value[4], value[5]);
    return Glib::ustring(buf);
}


bool MacAddress::parse(const char* spec)
{
    unsigned char a[6];
    const char* s = spec;
    char t[4] = { '\0', '\0', '\0', '\0' };
    if (isxdigit(*s))
        t[0] = *s++;
    else
        return false;
    if (isxdigit(*s))
        t[1] = *s++;
    else
        return false;
    a[0] = (unsigned char)strtoul(t, NULL, 16);
    for (int i = 1; i < 6; i++)
    {
        if (*s == '-' || *s == ':')
            s++;
        if (isxdigit(*s))
            t[0] = *s++;
        else
            return false;
        if (isxdigit(*s))
            t[1] = *s++;
        else
            return false;
        a[i] = (unsigned char)strtoul(t, NULL, 16);
    }
    if (*s)
        return false;
    memcpy(value, a, 6);
    return true;
}


bool MacAddress::getByName(const char* name)
{
    HostEntry ent(name);
    if (ent.isValid())
    {
        AddressResolution resolver;
        for (HostEntry::Iter iter = ent.begin(); iter != ent.end(); iter++)
        {
            in_addr_t ip = *iter;
            int i = resolver.getByIpAddress(ip);
            if (i >= 0)
            {
                memcpy(value, resolver[i].mac, 6);
                return true;
            }
        }
    }
    return false;
}


MacAddress& MacAddress::operator =(const MacAddress& rhs)
{
    memcpy(value, rhs.value, 6);
    return *this;
}
