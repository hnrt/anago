// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_OPTICALDISK_H
#define HNRT_OPTICALDISK_H


#include <list>
#include <glibmm/ustring.h>


namespace hnrt
{
    struct OpticalDisk
    {
        Glib::ustring vdi;
        Glib::ustring name;

        inline OpticalDisk();
        inline OpticalDisk(const Glib::ustring& vdi_, const Glib::ustring& name_);
        inline OpticalDisk(const OpticalDisk& src);
        inline OpticalDisk& operator =(const OpticalDisk& rhs);
    };

    inline OpticalDisk::OpticalDisk()
    {
    }

    inline OpticalDisk::OpticalDisk(const Glib::ustring& vdi_, const Glib::ustring& name_)
        : vdi(vdi_)
        , name(name_)
    {
    }

    inline OpticalDisk::OpticalDisk(const OpticalDisk& src)
        : vdi(src.vdi)
        , name(src.name)
    {
    }

    inline OpticalDisk& OpticalDisk::operator =(const OpticalDisk& rhs)
    {
        vdi = rhs.vdi;
        name = rhs.name;
        return *this;
    }

    struct OpticalDiskList
        : public std::list<OpticalDisk>
    {
        inline OpticalDiskList();
        inline void insert(const Glib::ustring&, const Glib::ustring&);

        typedef std::list<OpticalDisk> Super;
        typedef std::list<OpticalDisk>::iterator Iter;
        typedef std::list<OpticalDisk>::const_iterator ConstIter;
    };

    inline OpticalDiskList::OpticalDiskList()
    {
    }

    inline void OpticalDiskList::insert(const Glib::ustring& vdi, const Glib::ustring& name)
    {
        for (Iter iter = Super::begin(); iter != Super::end() ; iter++)
        {
            if (iter->name > name)
            {
                Super::insert(iter, OpticalDisk(vdi, name));
                return;
            }
        }
        push_back(OpticalDisk(vdi, name));
    }
}


#endif //!HNRT_OPTICALDISK_H
