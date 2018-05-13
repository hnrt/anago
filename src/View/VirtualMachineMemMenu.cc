// Copyright (C) 2012-2018 Hideaki Narita


#include <libintl.h>
#include "Controller/Controller.h"
#include "XenServer/VirtualMachine.h"
#include "VirtualMachineMemMenu.h"


using namespace hnrt;


VirtualMachineMemMenu::VirtualMachineMemMenu(const RefPtr<VirtualMachine>& vm)
    : _menuChange(gettext("Change"))
    , _menuCancel(gettext("Cancel"))
    , _vm(vm)
{
    append(_menuChange);
    append(*Gtk::manage(new Gtk::SeparatorMenuItem));
    append(_menuCancel);
    show_all_children();
    signal_deactivate().connect(sigc::mem_fun(*this, &VirtualMachineMemMenu::onDeactivate));
    signal_selection_done().connect(sigc::mem_fun(*this, &VirtualMachineMemMenu::onSelectionDone));
    _menuChange.signal_activate().connect(sigc::mem_fun(*this, &VirtualMachineMemMenu::onChange));
}


void VirtualMachineMemMenu::popup(guint button, guint32 activateTime)
{
    popup(button, activateTime, Glib::ustring());
}


void VirtualMachineMemMenu::popup(guint button, guint32 activateTime, const Glib::ustring& name)
{
    _name = name;

    bool sensitive = false;
    if (_name.empty() ||
        _name == "memory_static_max" ||
        _name == "memory_static_min" ||
        _name == "memory_dynamic_max" ||
        _name == "memory_dynamic_min"
        )
    {
        XenPtr<xen_vm_record> record = _vm->getRecord();
        if (record->power_state == XEN_VM_POWER_STATE_HALTED)
        {
            sensitive = true;
        }
    }
    _menuChange.set_sensitive(sensitive);
    Gtk::Menu::popup(button, activateTime);
}


void VirtualMachineMemMenu::onDeactivate()
{
    // This is called right after the closure of the popup window.
}


void VirtualMachineMemMenu::onSelectionDone()
{
    // This is called after all the things are done.
}


void VirtualMachineMemMenu::onChange()
{
    if (_name.empty() ||
        _name == "memory_static_max" ||
        _name == "memory_static_min" ||
        _name == "memory_dynamic_max" ||
        _name == "memory_dynamic_min"
        )
    {
        Controller::instance().changeMemory();
    }
}
