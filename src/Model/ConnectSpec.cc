// Copyright (C) 2012-2018 Hideaki Narita


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "Base/StringBuffer.h"
#include "File/Json.h"
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


ConnectSpec& ConnectSpec::operator =(const ConnectSpec& src)
{
    uuid = src.uuid;
    displayname = src.displayname;
    hostname = src.hostname;
    username = src.username;
    password = src.password;
    lastAccess = src.lastAccess;
    autoConnect = src.autoConnect;
    mac = src.mac;
    displayOrder = src.displayOrder;
    return *this;
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


bool ConnectSpec::fromJson(const RefPtr<Json>& value)
{
    Glib::ustring tmp;
    if (value->get("uuid", uuid) &&
        value->get("display_name", displayname) &&
        value->get("host", hostname) &&
        value->get("user", username) &&
        value->get("password", password) &&
        value->get("last_access", lastAccess) &&
        value->get("auto_connect", autoConnect) &&
        value->get("mac", tmp) &&
        value->get("display_order", displayOrder))
    {
        mac.parse(tmp.c_str());
        return true;
    }
    else
    {
        return false;
    }
}


RefPtr<Json> ConnectSpec::toJson() const
{
    RefPtr<Json> value = Json::create(Json::OBJECT);
    value->set("uuid", uuid);
    value->set("display_name", displayname);
    value->set("host", hostname);
    value->set("user", username);
    value->set("password", password);
    value->set("last_access", lastAccess);
    value->set("auto_connect", autoConnect);
    value->set("mac", mac.toString());
    value->set("display_order", displayOrder);
    return value;
}
