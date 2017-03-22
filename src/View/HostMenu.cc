// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Controller/Controller.h"
#include "XenServer/Host.h"
#include "XenServer/Session.h"
#include "XenServer/XenObjectStore.h"
#include "HostMenu.h"


using namespace hnrt;


HostMenu::HostMenu(Type type)
    : _menuConnect(gettext("Connect"))
    , _menuDisconnect(gettext("Disconnect"))
    , _menuWake(gettext("Power on"))
    , _menuShutdown(gettext("Shut down"))
    , _menuRestart(gettext("Restart"))
    , _menuChangeName(gettext("Label/Description"))
    , _menuAddVm(gettext("Add VM"))
    , _menuAddCifs(gettext("Add CIFS"))
    , _menuEditConnectSpec(gettext("Edit"))
    , _menuRemove(gettext("Remove"))
    , _menuChange(gettext("Change"))
    , _menuCancel(gettext("Cancel"))
{
    if (type == DEFAULT)
    {
        append(_menuConnect);
        append(_menuDisconnect);
        append(_menuWake);
        append(_menuShutdown);
        append(_menuRestart);
        append(_menuChange);
        append(_menuAddVm);
        append(_menuAddCifs);
        append(_menuEditConnectSpec);
        append(_menuRemove);
        append(*Gtk::manage(new Gtk::SeparatorMenuItem));
        append(_menuCancel);
        _submenuChange.append(_menuChangeName);
        _menuChange.set_submenu(_submenuChange);
        show_all_children();
        signal_deactivate().connect(sigc::mem_fun(*this, &HostMenu::onDeactivate));
        signal_selection_done().connect(sigc::mem_fun(*this, &HostMenu::onSelectionDone));
        _menuConnect.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::connect));
        _menuDisconnect.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::disconnect));
        _menuAddVm.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::addVm));
        _menuAddCifs.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::addCifs));
        _menuWake.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::wakeHost));
        _menuShutdown.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::shutdownHosts));
        _menuRestart.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::restartHosts));
        _menuEditConnectSpec.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::editHost));
        _menuRemove.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::removeHost));
        _menuChangeName.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::changeHostName));
    }
    else if (type == NAME_VALUE)
    {
        append(_menuChange);
        append(*Gtk::manage(new Gtk::SeparatorMenuItem));
        append(_menuCancel);
        show_all_children();
        signal_deactivate().connect(sigc::mem_fun(*this, &HostMenu::onDeactivate));
        signal_selection_done().connect(sigc::mem_fun(*this, &HostMenu::onSelectionDone));
        _menuChange.signal_activate().connect(sigc::mem_fun(*this, &HostMenu::onChange));
    }
}


HostMenu::~HostMenu()
{
}


void HostMenu::popup(guint button, guint32 activateTime, const RefPtr<Host> host)
{
    bool sensitive = host->isBusy() ? false : true;
    if (host->getSession().isConnected())
    {
        _menuConnect.hide();
        _menuWake.hide();
        _menuDisconnect.show();
        _menuShutdown.show();
        _menuRestart.show();
        _menuChange.show();
        _menuAddVm.show();
        _menuAddCifs.show();
        _menuDisconnect.set_sensitive(sensitive);
        _menuShutdown.set_sensitive(sensitive);
        _menuRestart.set_sensitive(sensitive);
        _menuChange.set_sensitive(sensitive);
        _menuAddVm.set_sensitive(sensitive);
        _menuAddCifs.set_sensitive(sensitive);
        _menuEditConnectSpec.hide();
        _menuRemove.hide();
    }
    else
    {
        _menuConnect.show();
        _menuWake.show();
        _menuDisconnect.hide();
        _menuShutdown.hide();
        _menuRestart.hide();
        _menuChange.hide();
        _menuAddVm.hide();
        _menuAddCifs.hide();
        _menuConnect.set_sensitive(sensitive);
        //_menuWake.set_sensitive(!host->getPing());
        _menuEditConnectSpec.show();
        _menuRemove.show();
    }
    Gtk::Menu::popup(button, activateTime);
}


void HostMenu::popup(guint button, guint32 activateTime, const Glib::ustring& name)
{
    _name = name;

    bool sensitive = false;
    if (
        _name == "Name" ||
        _name == "Description"
        )
    {
        sensitive = true;
    }
    _menuChange.set_sensitive(sensitive);
    Gtk::Menu::popup(button, activateTime);
}


void HostMenu::onDeactivate()
{
    // This is called right after the closure of the popup window.
}


void HostMenu::onSelectionDone()
{
    // This is called after all the things are done.
}


void HostMenu::onChange()
{
    if (
        _name == "Name" ||
        _name == "Description"
        )
    {
        Controller::instance().changeHostName();
    }
}
