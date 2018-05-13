// Copyright (C) 2012-2018 Hideaki Narita


#include "File/Json.h"
#include "Net/Console.h"
#include "ConsoleInfo.h"


using namespace hnrt;


ConsoleInfo::ConsoleInfo()
    : uuid()
    , enabled(true)
    , scale(false)
{
}


ConsoleInfo::ConsoleInfo(const Glib::ustring& uuid_)
    : uuid(uuid_)
    , enabled(true)
    , scale(false)
{
}


ConsoleInfo::ConsoleInfo(const ConsoleInfo& src)
    : uuid(src.uuid)
    , enabled(src.enabled)
    , scale(src.scale)
    , console(src.console)
{
}


void ConsoleInfo::assign(const ConsoleInfo& src)
{
    uuid = src.uuid;
    enabled = src.enabled;
    scale = src.scale;
    console = src.console;
}


bool ConsoleInfo::fromJson(const RefPtr<Json>& value)
{
    if (value->get("uuid", uuid) &&
        value->get("enabled", enabled) &&
        value->get("scale", scale))
    {
        return true;
    }
    else
    {
        return false;
    }
}


RefPtr<Json> ConsoleInfo::toJson() const
{
    RefPtr<Json> value = Json::create(Json::OBJECT);
    value->set("uuid", uuid);
    value->set("enabled", enabled);
    value->set("scale", scale);
    return value;
}
