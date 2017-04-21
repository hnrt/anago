// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_OPTICALDEVICE_H
#define HNRT_OPTICALDEVICE_H


#include <glibmm/ustring.h>
#include <list>


namespace hnrt
{
    struct OpticalDevice
    {
        Glib::ustring vbd;
        Glib::ustring name;
        Glib::ustring vdi;

        inline OpticalDevice();
        inline OpticalDevice(const Glib::ustring&, const Glib::ustring&, const Glib::ustring&);
        inline OpticalDevice(const OpticalDevice&);
        inline void operator =(const OpticalDevice&);
    };

    inline OpticalDevice::OpticalDevice()
    {
    }

    inline OpticalDevice::OpticalDevice(const Glib::ustring& vbd_, const Glib::ustring& name_, const Glib::ustring& vdi_)
        : vbd(vbd_)
        , name(name_)
        , vdi(vdi_)
    {
    }

    inline OpticalDevice::OpticalDevice(const OpticalDevice& src)
        : vbd(src.vbd)
        , name(src.name)
        , vdi(src.vdi)
    {
    }

    inline void OpticalDevice::operator =(const OpticalDevice& src)
    {
        vbd = src.vbd;
        name = src.name;
        vdi = src.vdi;
    }

    struct OpticalDeviceList
        : public std::list<OpticalDevice>
    {
        inline OpticalDeviceList();
        inline void insert(const Glib::ustring&, const Glib::ustring&, const Glib::ustring&);

        typedef std::list<OpticalDevice> Super;
        typedef std::list<OpticalDevice>::iterator Iter;
        typedef std::list<OpticalDevice>::const_iterator ConstIter;
    };

    inline OpticalDeviceList::OpticalDeviceList()
    {
    }

    inline void OpticalDeviceList::insert(const Glib::ustring& vbd, const Glib::ustring& name, const Glib::ustring& vdi)
    {
        for (Iter iter = Super::begin(); iter != Super::end(); iter++)
        {
            if (iter->name > name)
            {
                Super::insert(iter, OpticalDevice(vbd, name, vdi));
                return;
            }
        }
        Super::push_back(OpticalDevice(vbd, name, vdi));
    }
}


#endif //!HNRT_OPTICALDEVICE_H
