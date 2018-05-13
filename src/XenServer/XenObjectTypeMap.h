// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_XENOBJECTTYPEMAP_H
#define HNRT_XENOBJECTTYPEMAP_H


#include "XenObject.h"


namespace hnrt
{
    struct XenObjectTypeMap
    {
        static void init();
        static void fini();
        static XenObject::Type find(const char*);
        static const char* toString(XenObject::Type);
    };
}


#endif //!HNRT_XENOBJECTTYPEMAP_H
