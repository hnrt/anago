// Copyright (C) 2012-2018 Hideaki Narita


#include <libintl.h>
#include "Controller/Controller.h"
#include "XenServer/Network.h"
#include "NetworkMenu.h"


using namespace hnrt;


NetworkMenu::NetworkMenu(const RefPtr<Network>& network)
    : _menuChange(gettext("Change"))
    , _menuCancel(gettext("Cancel"))
    , _network(network)
    , _readonly(network->isHostInternalManagement())
{
    init();
}


NetworkMenu::NetworkMenu()
    : _menuChange(gettext("Change"))
    , _menuCancel(gettext("Cancel"))
    , _readonly(false)
{
    init();
}


void NetworkMenu::init()
{
    append(_menuChange);
    append(*Gtk::manage(new Gtk::SeparatorMenuItem));
    append(_menuCancel);
    show_all_children();
    signal_deactivate().connect(sigc::mem_fun(*this, &NetworkMenu::onDeactivate));
    signal_selection_done().connect(sigc::mem_fun(*this, &NetworkMenu::onSelectionDone));
    _menuChange.signal_activate().connect(sigc::mem_fun(*this, &NetworkMenu::onChange));
}


void NetworkMenu::popup(guint button, guint32 activateTime, const Glib::ustring& name)
{
    _name = name;

    bool sensitive = false;
    if (
        _name == "Label" ||
        _name == "Description"
        )
    {
        if (!_readonly)
        {
            sensitive = true;
        }
    }
    _menuChange.set_sensitive(sensitive);
    Gtk::Menu::popup(button, activateTime);
}


void NetworkMenu::popup(guint button, guint32 activateTime, Network& network)
{
    _network = RefPtr<Network>(&network, 1);
    _readonly = network.isHostInternalManagement();
    _name = "";
    _menuChange.set_sensitive(!_readonly);
    Gtk::Menu::popup(button, activateTime);
}


void NetworkMenu::onDeactivate()
{
    // This is called right after the closure of the popup window.
}


void NetworkMenu::onSelectionDone()
{
    // This is called after all the things are done.
}


void NetworkMenu::onChange()
{
    if (
        _name == "Label" ||
        _name == "Description"
        )
    {
        Controller::instance().changeNetworkName(*_network);
    }
    else if (_name.empty())
    {
        Controller::instance().changeNetworkName(*_network);
        _network.reset();
    }
}
