// Copyright (C) 2012-2018 Hideaki Narita


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
        Glib::ustring label;
        Glib::ustring description;
        bool sharable;
        bool readonly;

        HardDiskDriveSpec();
        HardDiskDriveSpec(const HardDiskDriveSpec&);
        void operator =(const HardDiskDriveSpec&);
        Glib::ustring getSrName(const Session&) const;
    };
}


#endif //!HNRT_HARDDISKDRIVESPEC_H
