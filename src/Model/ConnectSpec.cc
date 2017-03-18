// Copyright (C) 2012-2017 Hideaki Narita


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "Base/StringBuffer.h"
#include "Util/Base64.h"
#include "Util/Scrambler.h"
#include "Util/UUID.h"
#include "ConnectSpec.h"


using namespace hnrt;


ConnectSpec::ConnectSpec()
    : uuid()
    , displayname()
    , hostname()
    , username()
    , password()
    , lastAccess(0L)
    , autoConnect(false)
    , mac()
    , displayOrder(INT_MAX)
{
}


ConnectSpec::ConnectSpec(const ConnectSpec& src)
    : uuid(src.uuid)
    , displayname(src.displayname)
    , hostname(src.hostname)
    , username(src.username)
    , password(src.password)
    , lastAccess(src.lastAccess)
    , autoConnect(src.autoConnect)
    , mac(src.mac)
    , displayOrder(src.displayOrder)
{
}


ConnectSpec& ConnectSpec::operator =(const ConnectSpec& rhs)
{
    uuid = rhs.uuid;
    displayname = rhs.displayname;
    hostname = rhs.hostname;
    username = rhs.username;
    password = rhs.password;
    lastAccess = rhs.lastAccess;
    autoConnect = rhs.autoConnect;
    mac = rhs.mac;
    displayOrder = rhs.displayOrder;
    return *this;
}


Glib::ustring ConnectSpec::toString() const
{
    return Glib::ustring::compose("%1,%2,%3,%4,%5,%6,%7,%8,%9",
                                  uuid,
                                  displayname,
                                  hostname,
                                  username,
                                  password,
                                  lastAccess,
                                  autoConnect ? 1 : 0,
                                  mac.toString(),
                                  displayOrder);
}


bool ConnectSpec::parse(int version, const char* s, ConnectSpec& cs)
{
    const char *t = strchr(s, ',');
    if (version >= 2)
    {
        if (t)
        {
            cs.uuid.assign(s, t - s);
            s = t + 1;
        }
        else
        {
            cs.uuid = s;
            fprintf(stderr, "Error: Missing display host name.\n");
            return false;
        }

        t = strchr(s, ',');
        if (t)
        {
            cs.displayname.assign(s, t - s);
            s = t + 1;
        }
        else
        {
            cs.displayname = s;
            fprintf(stderr, "Error: Missing host name.\n");
            return false;
        }

        t = strchr(s, ',');
        if (t)
        {
            cs.hostname.assign(s, t - s);
            s = t + 1;
        }
        else
        {
            cs.hostname = s;
            fprintf(stderr, "Error: Missing user name.\n");
            return false;
        }
    }
    else
    {
        if (t)
        {
            cs.uuid.assign(s, t - s);
            s = t + 1;
        }
        else
        {
            cs.uuid = s;
            fprintf(stderr, "Error: Missing host name.\n");
            return false;
        }

        t = strchr(s, ',');
        if (t)
        {
            cs.displayname = cs.hostname.assign(s, t - s);
            s = t + 1;
        }
        else
        {
            cs.displayname = cs.hostname = s;
            fprintf(stderr, "Error: Missing user name.\n");
            return false;
        }
    }

    t = strchr(s, ',');
    if (t)
    {
        cs.username.assign(s, t - s);
        s = t + 1;
    }
    else
    {
        cs.username = s;
        fprintf(stderr, "Error: Missing password.\n");
        return false;
    }

    t = strchr(s, ',');
    if (t)
    {
        cs.password.assign(s, t - s);
        s = t + 1;
    }
    else
    {
        cs.password = s;
        fprintf(stderr, "Error: Missing last access.\n");
        return false;
    }

    cs.lastAccess = strtol(s, (char**)&t, 10);
    if (s < t && *t == ',')
    {
        s = t + 1;
    }
    else
    {
        fprintf(stderr, "Error: Malformed last access.\n");
        return false;
    }

    if (version < 3)
    {
        cs.autoConnect = strtol(s, (char**)&t, 0) ? true : false;
        if (s < t && (*t == ',' || *t == '\0'))
        {
            if (*t == ',')
            {
                s = t + 1;
            }
            else
            {
                return true;
            }
        }
        else
        {
            fprintf(stderr, "Error: Malformed auto connect.\n");
            return false;
        }

        if (cs.mac.parse(s))
        {
            return true;
        }
        else
        {
            fprintf(stderr, "Error: Malformed MAC address.\n");
            return false;
        }
    }

    cs.autoConnect = strtol(s, (char**)&t, 0) ? true : false;
    if (s < t && *t == ',')
    {
        s = t + 1;
    }
    else
    {
        fprintf(stderr, "Error: Malformed auto connect.\n");
        return false;
    }

    t = strchr(s, ',');
    if (t)
    {
        Glib::ustring x(s, t - s);
        if (!cs.mac.parse(x.c_str()))
        {
            fprintf(stderr, "Error: Malformed MAC address.\n");
            return false;
        }
        s = t + 1;
    }
    else
    {
        fprintf(stderr, "Error: Missing display order.\n");
        return false;
    }

    cs.displayOrder = strtol(s, (char**)&t, 0);
    if (s < t && *t == '\0')
    {
        return true;
    }
    else
    {
        fprintf(stderr, "Error: Malformed display order.\n");
        return false;
    }
}


Glib::ustring ConnectSpec::descramblePassword() const
{
    Base64Decoder d1(password.c_str());
    Descrambler d2(d1.getValue(), d1.getLength());
    return Glib::ustring(reinterpret_cast<const char*>(d2.getValue()));
}


Glib::ustring ConnectSpec::getBasicAuthString() const
{
    Base64Decoder d1(password.c_str());
    Descrambler d2(d1.getValue(), d1.getLength());
    StringBuffer d3;
    d3.format("%s:%s", username.c_str(), reinterpret_cast<const char*>(d2.getValue()));
    Base64Encoder auth(d3.ptr(), d3.len());
    d3.clear();
    d2.clear();
    d1.clear();
    return Glib::ustring(auth);
}
