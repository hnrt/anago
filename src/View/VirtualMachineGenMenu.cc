// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Controller/Controller.h"
#include "XenServer/VirtualMachine.h"
#include "VirtualMachineGenMenu.h"


using namespace hnrt;


VirtualMachineGenMenu::VirtualMachineGenMenu(const RefPtr<VirtualMachine>& vm)
    : _menuChange(gettext("Change"))
    , _menuChangeName(gettext("Change name"))
    , _menuChangeCpu(gettext("Change CPU"))
    , _menuChangeShadowMemory(gettext("Change shadow memory"))
    , _menuChangeVga(gettext("Change VGA"))
    , _menuAttachHdd(gettext("Attach HDD"))
    , _menuAttachCd(gettext("Attach CD drive"))
    , _menuAttachNic(gettext("Attach NIC"))
    , _menuCancel(gettext("Cancel"))
    , _vm(vm)
{
    append(_menuChange);
    append(_menuChangeName);
    append(_menuChangeCpu);
    append(_menuChangeShadowMemory);
    append(_menuChangeVga);
    append(_menuAttachHdd);
    append(_menuAttachCd);
    append(_menuAttachNic);
    append(*Gtk::manage(new Gtk::SeparatorMenuItem));
    append(_menuCancel);
    show_all_children();
    signal_deactivate().connect(sigc::mem_fun(*this, &VirtualMachineGenMenu::onDeactivate));
    signal_selection_done().connect(sigc::mem_fun(*this, &VirtualMachineGenMenu::onSelectionDone));
    _menuChange.signal_activate().connect(sigc::mem_fun(*this, &VirtualMachineGenMenu::onChange));
    _menuChangeName.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::changeVmName));
    _menuChangeCpu.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::changeCpu));
    _menuChangeShadowMemory.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::changeShadowMemory));
    _menuChangeVga.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::changeVga));
    _menuAttachHdd.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::attachHdd));
    _menuAttachCd.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::attachCd));
    _menuAttachNic.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::attachNic));
}


void VirtualMachineGenMenu::popup(guint button, guint32 activeTime)
{
    popup(button, activeTime, Glib::ustring());
}


void VirtualMachineGenMenu::popup(guint button, guint32 activateTime, const Glib::ustring& name)
{
    _name = name;

    bool sensitive = false;
    XenPtr<xen_vm_record> record = _vm->getRecord();
    if (record->power_state == XEN_VM_POWER_STATE_HALTED)
    {
        sensitive = true;
    }
    if (_name.empty())
    {
        _menuChange.hide();
        _menuChangeName.show();
        _menuChangeCpu.show();
        _menuChangeShadowMemory.show();
        _menuChangeVga.show();
        _menuChangeCpu.set_sensitive(sensitive);
        _menuChangeShadowMemory.set_sensitive(sensitive);
        _menuChangeVga.set_sensitive(sensitive);
        _menuAttachHdd.set_sensitive(sensitive);
        _menuAttachCd.set_sensitive(sensitive);
        _menuAttachNic.set_sensitive(sensitive);
    }
    else
    {
        _menuChange.show();
        _menuChangeName.hide();
        _menuChangeCpu.hide();
        _menuChangeShadowMemory.hide();
        _menuChangeVga.hide();
        _menuAttachHdd.hide();
        _menuAttachCd.hide();
        _menuAttachNic.hide();
        if (
            _name == "Name" ||
            _name == "Description"
            )
        {
            sensitive = true;
        }
        else if (
            _name == "vcpus_max" ||
            _name == "vcpus_at_startup" ||
            _name == "hvm_shadow_multiplier" ||
            _name == "platform: cores-per-socket" ||
            _name == "platform: videoram" ||
            _name == "platform: vga"
            )
        {
        }
        else
        {
            sensitive = false;
        }
        _menuChange.set_sensitive(sensitive);
    }
    Gtk::Menu::popup(button, activateTime);
}


void VirtualMachineGenMenu::onDeactivate()
{
    // This is called right after the closure of the popup window.
}


void VirtualMachineGenMenu::onSelectionDone()
{
    // This is called after all the things are done.
}


void VirtualMachineGenMenu::onChange()
{
    if (
        _name == "Name" ||
        _name == "Description"
        )
    {
        Controller::instance().changeVmName();
    }
    else if (
        _name == "vcpus_max" ||
        _name == "vcpus_at_startup" ||
        _name == "platform: cores-per-socket"
        )
    {
        Controller::instance().changeCpu();
    }
    else if (
        _name == "hvm_shadow_multiplier"
        )
    {
        Controller::instance().changeShadowMemory();
    }
    else if (
        _name == "platform: videoram" ||
        _name == "platform: vga"
        )
    {
        Controller::instance().changeVga();
    }
}
