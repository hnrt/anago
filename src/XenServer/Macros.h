// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_XENSERVER_MACROS_H
#define HNRT_XENSERVER_MACROS_H


#include "Constants.h"


#define IS_REF(x) (!strncmp(reinterpret_cast<const char*>(x),REFPREFIX,REFPREFIX_LENGTH))
#define IS_NULLREF(x) (!strcmp(reinterpret_cast<const char*>(x),NULLREFSTRING))


#endif //!HNRT_XENSERVER_MACROS_H
