// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_HARDDISKDRIVESPEC_H
#define HNRT_HARDDISKDRIVESPEC_H


#include <stdint.h>
#include <glibmm/ustring.h>


namespace hnrt
{
    class Session;

    struct HardDiskDriveSpec
    {
        Glib::ustring srREFID;
        int64_t size;
        Glib::ustring name;
        Glib::ustring description;

        HardDiskDriveSpec();
        HardDiskDriveSpec(const HardDiskDriveSpec& other);
        Glib::ustring getSrName(const Session& session) const;
    };
}


#endif //!HNRT_HARDDISKDRIVESPEC_H
