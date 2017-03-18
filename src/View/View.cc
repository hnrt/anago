// Copyright (C) 2012-2017 Hideaki Narita


#include "ViewImpl.h"


using namespace hnrt;


static ViewImpl* _singleton = NULL;


void View::init()
{
    _singleton = new ViewImpl();
}


void View::fini()
{
    delete _singleton;
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
