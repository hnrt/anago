// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Controller/Controller.h"
#include "XenServer/StorageRepository.h"
#include "StorageRepositoryMenu.h"


using namespace hnrt;


StorageRepositoryMenu::StorageRepositoryMenu()
    : _menuChange(gettext("Change"))
    , _menuAddHdd(gettext("Add HDD"))
    , _menuDeleteCifs(gettext("Delete"))
    , _menuCancel(gettext("Cancel"))
    , _menuChangeName(gettext("Label/Description"))
    , _menuSetDefault(gettext("Set default"))
{
    append(_menuChange);
    append(_menuAddHdd);
    append(_menuDeleteCifs);
    append(*Gtk::manage(new Gtk::SeparatorMenuItem));
    append(_menuCancel);
    _submenuChange.append(_menuChangeName); 
    _submenuChange.append(_menuSetDefault);
    _menuChange.set_submenu(_submenuChange);
    show_all_children();
    signal_deactivate().connect(sigc::mem_fun(*this, &StorageRepositoryMenu::onDeactivate));
    signal_selection_done().connect(sigc::mem_fun(*this, &StorageRepositoryMenu::onSelectionDone));
    _menuAddHdd.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::addHdd));
    _menuDeleteCifs.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::deleteCifs));
    _menuChangeName.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::changeSrName));
    _menuSetDefault.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::setDefaultSr));
}


void StorageRepositoryMenu::popup(guint button, guint32 activateTime, StorageRepository& sr)
{
    bool sensitive = sr.isBusy() ? false : true;
    if (sr.isTools())
    {
        _menuChange.set_sensitive(sensitive);
        _menuSetDefault.hide();
        _menuAddHdd.hide();
        _menuDeleteCifs.hide();
    }
    else if (sr.isCifs())
    {
        _menuChange.set_sensitive(sensitive);
        _menuSetDefault.hide();
        _menuAddHdd.hide();
        _menuDeleteCifs.show();
        _menuDeleteCifs.set_sensitive(sensitive);
    }
    else if (sr.getSubType() == StorageRepository::DEV)
    {
        _menuChange.set_sensitive(sensitive);
        _menuSetDefault.hide();
        _menuAddHdd.hide();
        _menuDeleteCifs.hide();
    }
    else
    {
        _menuChange.set_sensitive(sensitive);
        _menuSetDefault.show();
        _menuSetDefault.set_sensitive(sr.isDefault() ? false : true);
        _menuAddHdd.show();
        _menuAddHdd.set_sensitive(sensitive);
        _menuDeleteCifs.hide();
    }
    Gtk::Menu::popup(button, activateTime);
}


void StorageRepositoryMenu::popup(guint button, guint32 activateTime, const Glib::ustring& name)
{
    if (name == "Name" ||
        name == "Description")
    {
        _menuChange.set_sensitive(true);
    }
    else
    {
        _menuChange.set_sensitive(false);
    }
    _menuSetDefault.hide();
    _menuAddHdd.hide();
    _menuDeleteCifs.hide();
    Gtk::Menu::popup(button, activateTime);
}


void StorageRepositoryMenu::onDeactivate()
{
    // This is called right after the closure of the popup window.
}


void StorageRepositoryMenu::onSelectionDone()
{
    // This is called after all the things are done.
}
