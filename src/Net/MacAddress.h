// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_MACADDRESS_H
#define HNRT_MACADDRESS_H


#include <glibmm.h>


namespace hnrt
{
    struct MacAddress
    {
        unsigned char value[6];

        MacAddress();
        MacAddress(const MacAddress&);
        bool isNull() const;
        Glib::ustring toString() const;
        bool parse(const char* spec);
        bool getByName(const char* name);
        MacAddress& operator =(const MacAddress&);
    };
}


#endif //!HNRT_MACADDRESS_H
