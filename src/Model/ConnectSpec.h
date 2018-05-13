// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_CONNECTSPEC_H
#define HNRT_CONNECTSPEC_H


#include <glibmm/ustring.h>
#include "Base/RefPtr.h"
#include "Net/MacAddress.h"


namespace hnrt
{
    class Json;

    struct ConnectSpec
    {
        Glib::ustring uuid;
        Glib::ustring displayname;
        Glib::ustring hostname;
        Glib::ustring username;
        Glib::ustring password;
        long lastAccess;
        bool autoConnect;
        MacAddress mac;
        int displayOrder;

        ConnectSpec();
        ConnectSpec(const ConnectSpec&);
        ConnectSpec& operator =(const ConnectSpec&);
        Glib::ustring descramblePassword() const;
        Glib::ustring getBasicAuthString() const;
        bool fromJson(const RefPtr<Json>&);
        RefPtr<Json> toJson() const;
    };
}


#endif //!HNRT_CONNECTSPEC_H
