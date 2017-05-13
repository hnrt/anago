// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Constants.h"
#include "HardDiskDriveSpec.h"
#include "Session.h"
#include "StorageRepository.h"
#include "XenObjectStore.h"


using namespace hnrt;


HardDiskDriveSpec::HardDiskDriveSpec()
    : srREFID(NULLREFSTRING)
    , size(1024L * 1024L * 1024L)
    , label(gettext("New hard disk drive"))
    , description(gettext("Created by Anago"))
    , sharable(false)
    , readonly(false)
{
}


HardDiskDriveSpec::HardDiskDriveSpec(const HardDiskDriveSpec& src)
    : srREFID(src.srREFID)
    , size(src.size)
    , label(src.label)
    , description(src.description)
    , sharable(src.sharable)
    , readonly(src.readonly)
{
}


void HardDiskDriveSpec::operator =(const HardDiskDriveSpec& src)
{
    srREFID = src.srREFID;
    size = src.size;
    label = src.label;
    description = src.description;
    sharable = src.sharable;
    readonly = src.readonly;
}


Glib::ustring HardDiskDriveSpec::getSrName(const Session& session) const
{
    RefPtr<StorageRepository> sr = session.getStore().getSr(srREFID);
    return sr ? sr->getName() : Glib::ustring();
}
