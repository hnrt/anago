// Copyright (C) 2012-2017 Hideaki Narita


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include "NetRoute.h"


using namespace hnrt;


NetRoute::NetRoute()
    : _base(0)
    , _size(0)
    , _capacity(0)
{
    FILE* fp = NULL;
    try
    {
        fp = fopen("/proc/net/route", "r");
        if (!fp)
        {
            return;
        }
        char buf[512];
        while (fgets(buf, sizeof(buf), fp))
        {
            Record r;
            if (!r.parse(buf))
            {
                continue;
            }
            if (_size == _capacity)
            {
                int c = _capacity ? _capacity * 2 : 64;
                Record* b = (Record*)realloc(_base, c * sizeof(Record));
                if (!b)
                {
                    throw std::bad_alloc();
                }
                _base = b;
                _capacity = c;
            }
            new(_base + _size) Record(r);
            _size++;
        }
        fclose(fp);
    }
    catch (...)
    {
        while (_size)
        {
            (_base + --_size)->~Record();
        }
        free(_base);
        if (fp)
        {
            fclose(fp);
        }
        throw;
    }
}


NetRoute::NetRoute(const NetRoute& src)
    : _base(0)
    , _size(0)
    , _capacity(0)
{
    if (!src._size)
    {
        return;
    }
    _base = (Record*)malloc(src._size * sizeof(Record));
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


NetRoute::~NetRoute()
{
    while (_size)
    {
        (_base + --_size)->~Record();
    }
    free(_base);
}


in_addr_t NetRoute::getBroadcastAddress(in_addr_t a)
{
    for (int i = 0; i < _size; i++)
    {
        Record& r = *(_base + i);
        if (!r.mask)
        {
            continue;
        }
        in_addr_t b = a & r.mask;
        if (b == r.destination)
        {
            b |= ~r.mask;
            return b;
        }
    }
    return 0;
}


NetRoute& NetRoute::operator =(const NetRoute& rhs)
{
    if (_base)
    {
        while (_size)
        {
            (_base + --_size)->~Record();
        }
        free(_base);
        _base = NULL;
        _size = 0;
        _capacity = 0;
    }
    if (rhs._size)
    {
        _base = (Record*)malloc(rhs._size * sizeof(Record));
        if (!_base)
        {
            throw std::bad_alloc();
        }
        _capacity = rhs._size;
        while (_size < rhs._size)
        {
            new(_base + _size) Record(*(rhs._base + _size));
            _size++;
        }
    }
    return *this;
}


NetRoute::Record::Record()
    : name()
    , destination(0)
    , gateway(0)
    , flags(0)
    , refcnt(0)
    , use(0)
    , metric(0)
    , mask(0)
    , mtu(0)
    , window(0)
    , irtt(0)
{
}


NetRoute::Record::Record(const NetRoute::Record& src)
    : name(src.name)
    , destination(src.destination)
    , gateway(src.gateway)
    , flags(src.flags)
    , refcnt(src.refcnt)
    , use(src.use)
    , metric(src.metric)
    , mask(src.mask)
    , mtu(src.mtu)
    , window(src.window)
    , irtt(src.irtt)
{
}


NetRoute::Record::~Record()
{
}


bool NetRoute::Record::parse(const char* line)
{
    char* s = (char*)line;
    char* t = s;

    while (*t && !isspace(*t))
        t++;
    if (s < t)
    {
        name.assign(s, t - s);
        s = t;
    }
    else
        return false;

    while (isspace(*s))
        s++;

    destination = (unsigned int)strtoul(s, &t, 16);
    if (s < t)
        s = t;
    else
        return false;

    while (isspace(*s))
        s++;

    gateway = (unsigned int)strtoul(s, &t, 16);
    if (s < t)
        s = t;
    else
        return false;

    while (isspace(*s))
        s++;

    flags = (unsigned int)strtoul(s, &t, 16);
    if (s < t)
        s = t;
    else
        return false;

    while (isspace(*s))
        s++;

    refcnt = (unsigned int)strtoul(s, &t, 10);
    if (s < t)
        s = t;
    else
        return false;

    while (isspace(*s))
        s++;

    use = (unsigned int)strtoul(s, &t, 10);
    if (s < t)
        s = t;
    else
        return false;

    while (isspace(*s))
        s++;

    metric = (unsigned int)strtoul(s, &t, 10);
    if (s < t)
        s = t;
    else
        return false;

    while (isspace(*s))
        s++;

    mask = (unsigned int)strtoul(s, &t, 16);
    if (s < t)
        s = t;
    else
        return false;

    while (isspace(*s))
        s++;

    mtu = (unsigned int)strtoul(s, &t, 10);
    if (s < t)
        s = t;
    else
        return false;

    while (isspace(*s))
        s++;

    window = (unsigned int)strtoul(s, &t, 10);
    if (s < t)
        s = t;
    else
        return false;

    while (isspace(*s))
        s++;

    irtt = (unsigned int)strtoul(s, &t, 10);
    if (s < t)
        s = t;
    else
        return false;

    while (isspace(*s))
        s++;

    if (*s)
        return false;

    return true;
}
