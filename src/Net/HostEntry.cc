// Copyright (C) 2012-2017 Hideaki Narita


#include <string.h>
#include <errno.h>
#include "HostEntry.h"


using namespace hnrt;


HostEntry::HostEntry(const char* name)
    : buf(0)
{
    size_t bufsz = 256;
    buf = new char[bufsz];
    while (1)
    {
        struct hostent* phe = 0;
        int err = 0;
        int rc = gethostbyname2_r(name, AF_INET, &data, buf, bufsz, &phe, &err);
        if (rc == 0)
        {
            if (!phe)
            {
                memset(&data, 0, sizeof(data));
            }
            break;
        }
        else if (rc != ERANGE)
        {
            break;
        }
        delete[] buf;
        bufsz *= 2;
        buf = new char[bufsz];
    }
}


HostEntry::~HostEntry()
{
    delete[] buf;
}


bool HostEntry::isValid() const
{
    return data.h_addr_list ? true : false;
}


HostEntry::Iter HostEntry::begin() const
{
    return Iter(data.h_addr_list);
}


HostEntry::Iter HostEntry::end() const
{
    char** pp = data.h_addr_list;
    if (pp)
    {
        while (*pp)
        {
            pp++;
        }
    }
    return Iter(pp);
}


HostEntry::Iter::Iter(char** pp_)
    : pp(pp_)
{
}


HostEntry::Iter::Iter(const HostEntry::Iter& src)
    : pp(src.pp)
{
}


HostEntry::Iter::~Iter()
{
}


const in_addr_t& HostEntry::Iter::operator *() const
{
    return *reinterpret_cast<const in_addr_t*>(*pp);
}


HostEntry::Iter& HostEntry::Iter::operator =(const Iter& rhs)
{
    pp = rhs.pp;
    return *this;
}


bool HostEntry::Iter::operator ==(const Iter& rhs) const
{
    return pp == rhs.pp;
}


bool HostEntry::Iter::operator !=(const Iter& rhs) const
{
    return pp != rhs.pp;
}


HostEntry::Iter& HostEntry::Iter::operator ++()
{
    pp++;
    return *this;
}


HostEntry::Iter& HostEntry::Iter::operator ++(int)
{
    pp++;
    return *this;
}
