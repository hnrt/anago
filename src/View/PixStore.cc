// Copyright (C) 2012-2017 Hideaki Narita


#include "PixStore.h"
#include "XenServer/Host.h"
#include "XenServer/Session.h"


using namespace hnrt;


static PixStore* _singleton = NULL;


void PixStore::init()
{
    _singleton = new PixStore();
}


void PixStore::fini()
{
    delete _singleton;
}


PixStore& PixStore::instance()
{
    return *_singleton;
}


PixStore::PixStore()
    : _pixApp(Gdk::Pixbuf::create_from_file("/usr/share/icons/gnome/32x32/apps/preferences-desktop-remote-desktop.png"))
{
    Gtk::Invisible w;
    _pixYes = w.render_icon(Gtk::Stock::YES, Gtk::ICON_SIZE_SMALL_TOOLBAR);
    _pixNo = w.render_icon(Gtk::Stock::NO, Gtk::ICON_SIZE_SMALL_TOOLBAR);
}


Glib::RefPtr<Gdk::Pixbuf> PixStore::get(const RefPtr<Host> host)
{
    return
        !host ? _pixNo :
        host->isBusy() ? _pixYes :
        host->getSession().isConnected() ? _pixYes :
        _pixNo;
}
