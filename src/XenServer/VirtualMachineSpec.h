// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINESPEC_H
#define HNRT_VIRTUALMACHINESPEC_H


#include <glibmm/ustring.h>
#include <list>
#include "HardDiskDriveSpec.h"


namespace hnrt
{
    struct VirtualMachineSpec
    {
        Glib::ustring templateREFID;
        Glib::ustring name;
        Glib::ustring desc;
        std::list<HardDiskDriveSpec> hddList;
        Glib::ustring cdREFID;
        std::list<Glib::ustring> nwList;

        inline VirtualMachineSpec();
        inline VirtualMachineSpec(const VirtualMachineSpec& src);
        inline void operator =(const VirtualMachineSpec& rhs);
        inline void assign(const VirtualMachineSpec& other);
    };

    inline VirtualMachineSpec::VirtualMachineSpec()
    {
    }

    inline VirtualMachineSpec::VirtualMachineSpec(const VirtualMachineSpec& src)
    {
        assign(src);
    }

    inline void VirtualMachineSpec::operator =(const VirtualMachineSpec& src)
    {
        assign(src);
    }

    inline void VirtualMachineSpec::assign(const VirtualMachineSpec& src)
    {
        templateREFID = src.templateREFID;
        name = src.name;
        desc = src.desc;
        hddList = src.hddList;
        cdREFID = src.cdREFID;
        nwList = src.nwList;
    }
}


#endif //!HNRT_VIRTUALMACHINESPEC_H
