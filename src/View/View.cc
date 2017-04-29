// Copyright (C) 2012-2017 Hideaki Narita


#include "XenServer/XenObject.h"
#include "PixStore.h"
#include "ViewImpl.h"


using namespace hnrt;


static ViewImpl* _singleton = NULL;


void View::init()
{
    PixStore::init();

    _singleton = new ViewImpl();
}


void View::fini()
{
    delete _singleton;

    PixStore::fini();
}


View& View::instance()
{
    return *_singleton;
}


void View::update()
{
    while (Gtk::Main::events_pending())
    {
        Gtk::Main::iteration();
    }
}
