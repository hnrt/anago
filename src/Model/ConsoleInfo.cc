// Copyright (C) 2012-2017 Hideaki Narita


#include "Net/Console.h"
#include "ConsoleInfo.h"


using namespace hnrt;


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


void ConsoleInfo::assign(const ConsoleInfo& rhs)
{
    uuid = rhs.uuid;
    enabled = rhs.enabled;
    scale = rhs.scale;
    console = rhs.console;
}
