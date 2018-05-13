// Copyright (C) 2012-2018 Hideaki Narita


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <uuid/uuid.h>
#include "UUID.h"


using namespace hnrt;


Glib::ustring UUID::generate()
{
    uuid_t x = { 0 };
    uuid_generate(x);
    char buf[64];
    snprintf(buf, sizeof(buf),
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             x[3], x[2], x[1], x[0],
             x[5], x[4],
             x[7], x[6],
             x[8], x[9],
             x[10], x[11], x[12], x[13], x[14], x[15]);
    return Glib::ustring(buf);
}


UUID UUID::parse(const char* s, char** stopped)
{
    if (!stopped)
    {
        static char* unused;
        stopped = &unused;
    }
    UUID uuid;
    bool brace;
    if (*s == '{')
    {
        brace = true;
        s++;
    }
    else
    {
        brace = false;
    }
    for (int i = 0; i < 8; i++)
    {
        if (isxdigit(*s))
        {
            s++;
        }
        else
        {
            *stopped = (char*)s;
            throw std::invalid_argument("UUID::parse");
        }
    }
    if (*s == '-')
    {
        uuid.data1 = (unsigned int)strtoul(s - 8, NULL, 16);
        s++;
    }
    else
    {
        *stopped = (char*)s;
        throw std::invalid_argument("UUID::parse");
    }
    for (int i = 0; i < 4; i++)
    {
        if (isxdigit(*s))
        {
            s++;
        }
        else
        {
            *stopped = (char*)s;
            throw std::invalid_argument("UUID::parse");
        }
    }
    if (*s == '-')
    {
        uuid.data2 = (unsigned short)strtoul(s - 4, NULL, 16);
        s++;
    }
    else
    {
        *stopped = (char*)s;
        throw std::invalid_argument("UUID::parse");
    }
    for (int i = 0; i < 4; i++)
    {
        if (isxdigit(*s))
        {
            s++;
        }
        else
        {
            *stopped = (char*)s;
            throw std::invalid_argument("UUID::parse");
        }
    }
    if (*s == '-')
    {
        uuid.data3 = (unsigned short)strtoul(s - 4, NULL, 16);
        s++;
    }
    else
    {
        *stopped = (char*)s;
        throw std::invalid_argument("UUID::parse");
    }
    for (int i = 0; i < 4; i++)
    {
        if (isxdigit(*s))
        {
            s++;
        }
        else
        {
            *stopped = (char*)s;
            throw std::invalid_argument("UUID::parse");
        }
    }
    char buf[3];
    buf[0] = s[-4];
    buf[1] = s[-3];
    buf[2] = '\0';
    uuid.data4[0] = (unsigned char)strtoul(buf, NULL, 16);
    buf[0] = s[-2];
    buf[1] = s[-1];
    uuid.data4[1] = (unsigned char)strtoul(buf, NULL, 16);
    if (*s == '-')
    {
        s++;
    }
    else
    {
        *stopped = (char*)s;
        throw std::invalid_argument("UUID::parse");
    }
    for (int i = 0; i < 12; i++)
    {
        if (isxdigit(*s))
        {
            s++;
        }
        else
        {
            *stopped = (char*)s;
            throw std::invalid_argument("UUID::parse");
        }
    }
    buf[0] = s[-12];
    buf[1] = s[-11];
    uuid.data4[2] = (unsigned char)strtoul(buf, NULL, 16);
    buf[0] = s[-10];
    buf[1] = s[-9];
    uuid.data4[3] = (unsigned char)strtoul(buf, NULL, 16);
    buf[0] = s[-8];
    buf[1] = s[-7];
    uuid.data4[4] = (unsigned char)strtoul(buf, NULL, 16);
    buf[0] = s[-6];
    buf[1] = s[-5];
    uuid.data4[5] = (unsigned char)strtoul(buf, NULL, 16);
    buf[0] = s[-4];
    buf[1] = s[-3];
    uuid.data4[6] = (unsigned char)strtoul(buf, NULL, 16);
    buf[0] = s[-2];
    buf[1] = s[-1];
    uuid.data4[7] = (unsigned char)strtoul(buf, NULL, 16);
    if (brace)
    {
        if (*s == '}')
        {
            *stopped = (char*)s + 1;
        }
        else
        {
            *stopped = (char*)s;
            throw std::invalid_argument("UUID::parse");
        }
    }
    else
    {
        *stopped = (char*)s;
        if (isxdigit(*s))
        {
            throw std::invalid_argument("UUID::parse");
        }
    }
    return uuid;
}


UUID::UUID()
{
    memset(this, 0, 16);
}


UUID::UUID(const UUID& src)
{
    memcpy(this, &src, 16);
}


UUID::~UUID()
{
    memset(this, 0, 16);
}


UUID& UUID::set()
{
    uuid_t x = { 0 };
    uuid_generate(x);
    memcpy(this, x, 16);
    return *this;
}


UUID& UUID::operator =(const UUID& rhs)
{
    memcpy(this, &rhs, 16);
    return *this;
}


Glib::ustring UUID::toString() const
{
    char buf[64];
    snprintf(buf, sizeof(buf),
             "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             data1,
             data2,
             data3,
             data4[0], data4[1],
             data4[2], data4[3], data4[4], data4[5], data4[6], data4[7]);
    return Glib::ustring(buf);
}


bool UUID::operator ==(const UUID& rhs) const
{
    return memcmp(this, &rhs, 16) == 0 ? true : false;
}


bool UUID::operator !=(const UUID& rhs) const
{
    return memcmp(this, &rhs, 16) != 0 ? true : false;
}


bool UUID::isNull() const
{
    static const char zeros[16] = { 0 };
    return memcmp(this, zeros, 16) == 0 ? true : false;
}
