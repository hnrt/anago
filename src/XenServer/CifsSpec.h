// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_CIFSSPEC_H
#define HNRT_CIFSSPEC_H


#include <glibmm/ustring.h>


namespace hnrt
{
    struct CifsSpec
    {
        Glib::ustring label;
        Glib::ustring description;
        Glib::ustring location;
        Glib::ustring username;
        Glib::ustring password;

        CifsSpec();
        CifsSpec(const CifsSpec&);
        void operator =(const CifsSpec&);
    };
}


#endif //!HNRT_CIFSSPEC_H
