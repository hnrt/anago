// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Controller/Controller.h"
#include "XenServer/VirtualInterface.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/VirtualMachine.h"
#include "VirtualInterfaceMenu.h"


using namespace hnrt;


VirtualInterfaceMenu::VirtualInterfaceMenu()
    : _menuDetach(gettext("Detach"))
    , _menuCancel(gettext("Cancel"))
{
    append(_menuDetach);
    init();
}


VirtualInterfaceMenu::VirtualInterfaceMenu(const RefPtr<VirtualInterface>& vif)
    : _menuChange(gettext("Change"))
    , _menuCancel(gettext("Cancel"))
    , _vif(vif)
{
    append(_menuChange);
    init();
}


void VirtualInterfaceMenu::init()
{
    append(*Gtk::manage(new Gtk::SeparatorMenuItem));
    append(_menuCancel);
    show_all_children();
    signal_deactivate().connect(sigc::mem_fun(*this, &VirtualInterfaceMenu::onDeactivate));
    signal_selection_done().connect(sigc::mem_fun(*this, &VirtualInterfaceMenu::onSelectionDone));
    _menuChange.signal_activate().connect(sigc::mem_fun(*this, &VirtualInterfaceMenu::onChange));
    _menuDetach.signal_activate().connect(sigc::mem_fun(*this, &VirtualInterfaceMenu::onDetach));
}


void VirtualInterfaceMenu::popup(guint button, guint32 activateTime, const RefPtr<VirtualInterface>& vif)
{
    _vif = vif;
    if (!_vif)
    {
        return;
    }
    XenPtr<xen_vif_record> vifRecord = _vif->getRecord();
    RefPtr<VirtualMachine> vm = _vif->getVm();
    if (!vm)
    {
        return;
    }
    XenPtr<xen_vm_record> vmRecord = vm->getRecord();
    if (vmRecord->power_state == XEN_VM_POWER_STATE_HALTED)
    {
        _menuDetach.set_sensitive(true);
    }
    else
    {
        _menuDetach.set_sensitive(false);
    }
    Gtk::Menu::popup(button, activateTime);
}


void VirtualInterfaceMenu::popup(guint button, guint32 activateTime, const Glib::ustring& name)
{
    _name = name;
    _menuChange.set_sensitive(false);
    Gtk::Menu::popup(button, activateTime);
}


void VirtualInterfaceMenu::onDeactivate()
{
    // This is called right after the closure of the popup window.
}


void VirtualInterfaceMenu::onSelectionDone()
{
    // This is called after all the things are done.
    if (_name.empty())
    {
        _vif.reset();
    }
}


void VirtualInterfaceMenu::onChange()
{
}


void VirtualInterfaceMenu::onDetach()
{
    //Controller::instance().detachVif(_vif);
}
