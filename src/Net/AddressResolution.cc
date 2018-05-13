// Copyright (C) 2012-2018 Hideaki Narita


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include "AddressResolution.h"


using namespace hnrt;


AddressResolution::AddressResolution()
    : _base(0)
    , _size(0)
    , _capacity(0)
{
    FILE* fp = NULL;
    try
    {
        fp = fopen("/proc/net/arp", "r");
        if (!fp)
        {
            return;
        }
        char buf[256];
        while (fgets(buf, sizeof(buf), fp))
        {
            Record r;
            if (!r.parse(buf))
            {
                continue;
            }
            if (_size == _capacity)
            {
                _extend();
            }
            new(_base + _size++) Record(r);
        }
        fclose(fp);
    }
    catch (...)
    {
        _clear();
        if (fp)
        {
            fclose(fp);
        }
        throw;
    }
}


AddressResolution::AddressResolution(const AddressResolution& src)
    : _base(0)
    , _size(0)
    , _capacity(0)
{
    _assign(src);
}


AddressResolution::~AddressResolution()
{
    _clear();
}


AddressResolution& AddressResolution::assign(const AddressResolution& src)
{
    _clear();
    _base = NULL;
    _size = 0;
    _capacity = 0;
    _assign(src);
    return *this;
}


int AddressResolution::getByIpAddress(in_addr_t ip) const
{
    for (int i = 0; i < _size; i++)
    {
        if (!memcmp((_base + i)->ip, &ip, 4))
        {
            return i;
        }
    }
    return -1;
}


inline void AddressResolution::_clear()
{
    while (_size)
    {
        (_base + --_size)->~Record();
    }
    free(_base);
}


inline void AddressResolution::_extend()
{
    int c = _capacity ? _capacity * 2 : 64;
    Record* b = reinterpret_cast<Record*>(realloc(_base, c * sizeof(Record)));
    if (!b)
    {
        throw std::bad_alloc();
    }
    memset(b + _size, 0, (c - _size) * sizeof(Record));
    _base = b;
    _capacity = c;
}


inline void AddressResolution::_assign(const AddressResolution& src)
{
    if (!src._size)
    {
        return;
    }
    _base = reinterpret_cast<Record*>(malloc(src._size * sizeof(Record)));
    if (!_base)
    {
        throw std::bad_alloc();
    }
    _capacity = src._size;
    while (_size < src._size)
    {
        new(_base + _size) Record(*(src._base + _size));
        _size++;
    }
}


AddressResolution::Record::Record()
    : hwType(0)
    , flags(0)
{
    memset(ip, 0, sizeof(ip));
    memset(mac, 0, sizeof(mac));
}


AddressResolution::Record::Record(const AddressResolution::Record& src)
{
    _assign(src);
}


bool AddressResolution::Record::parse(const char* line)
{
    const char* s = line;
    const char* t = s;

    ip[0] = static_cast<unsigned char>(strtoul(s, &const_cast<char*&>(t), 10));
    if (s < t && *t == '.')
        s = t + 1;
    else
        return false;

    ip[1] = static_cast<unsigned char>(strtoul(s, &const_cast<char*&>(t), 10));
    if (s < t && *t == '.')
        s = t + 1;
    else
        return false;

    ip[2] = static_cast<unsigned char>(strtoul(s, &const_cast<char*&>(t), 10));
    if (s < t && *t == '.')
        s = t + 1;
    else
        return false;

    ip[3] = static_cast<unsigned char>(strtoul(s, &const_cast<char*&>(t), 10));
    if (s < t && isspace(*t))
        s = t + 1;
    else
        return false;

    while (isspace(*s))
        s++;

    hwType = static_cast<int>(strtoul(s, &const_cast<char*&>(t), 0));
    if (s < t && isspace(*t))
        s = t + 1;
    else
        return false;

    while (isspace(*s))
        s++;

    flags = static_cast<int>(strtoul(s, &const_cast<char*&>(t), 0));
    if (s < t && isspace(*t))
        s = t + 1;
    else
        return false;

    while (isspace(*s))
        s++;

    mac[0] = static_cast<unsigned char>(strtoul(s, &const_cast<char*&>(t), 16));
    if (s < t && *t == ':')
        s = t + 1;
    else
        return false;

    mac[1] = static_cast<unsigned char>(strtoul(s, &const_cast<char*&>(t), 16));
    if (s < t && *t == ':')
        s = t + 1;
    else
        return false;

    mac[2] = static_cast<unsigned char>(strtoul(s, &const_cast<char*&>(t), 16));
    if (s < t && *t == ':')
        s = t + 1;
    else
        return false;

    mac[3] = static_cast<unsigned char>(strtoul(s, &const_cast<char*&>(t), 16));
    if (s < t && *t == ':')
        s = t + 1;
    else
        return false;

    mac[4] = static_cast<unsigned char>(strtoul(s, &const_cast<char*&>(t), 16));
    if (s < t && *t == ':')
        s = t + 1;
    else
        return false;

    mac[5] = static_cast<unsigned char>(strtoul(s, &const_cast<char*&>(t), 16));
    if (s < t && isspace(*t))
        s = t + 1;
    else
        return false;

    while (isspace(*s))
        s++;

    t = s;
    while (*t && !isspace(*t))
        t++;

    if (s < t)
        mask.assign(s, t - s);
    else
        return false;

    s = t;
    while (isspace(*s))
        s++;

    t = s;
    while (*t && !isspace(*t))
        t++;

    if (s < t)
        device.assign(s, t - s);
    else
        return false;

    s = t;
    while (isspace(*s))
        s++;

    if (*s)
        return false;

    return true;
}


inline void AddressResolution::Record::_assign(const Record& src)
{
    hwType = src.hwType;
    flags = src.flags;
    mask = src.mask;
    device = src.mask;
    memcpy(ip, src.ip, sizeof(ip));
    memcpy(mac, src.mac, sizeof(mac));
}
