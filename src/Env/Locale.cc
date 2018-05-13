// Copyright (C) 2012-2018 Hideaki Narita


#include "LocaleImpl.h"


using namespace hnrt;


static LocaleImpl* _singleton = NULL;


void Locale::init()
{
    _singleton = new LocaleImpl();
}


void Locale::fini()
{
    delete _singleton;
}


Locale& Locale::instance()
{
    return *_singleton;
}
