// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Controller/Controller.h"
#include "XenServer/VirtualBlockDevice.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/VirtualMachine.h"
#include "VirtualBlockDeviceMenu.h"


using namespace hnrt;


VirtualBlockDeviceMenu::VirtualBlockDeviceMenu()
    : _menuDetach(gettext("Detach"))
    , _menuChangeCd(gettext("Change CD/DVD"))
    , _menuCancel(gettext("Cancel"))
{
    append(_menuDetach);
    append(_menuChangeCd);
    init();
}


VirtualBlockDeviceMenu::VirtualBlockDeviceMenu(const RefPtr<VirtualBlockDevice>& vbd)
    : _menuChange(gettext("Change"))
    , _menuCancel(gettext("Cancel"))
    , _vbd(vbd)
{
    append(_menuChange);
    init();
}


void VirtualBlockDeviceMenu::init()
{
    append(*Gtk::manage(new Gtk::SeparatorMenuItem));
    append(_menuCancel);
    show_all_children();
    signal_deactivate().connect(sigc::mem_fun(*this, &VirtualBlockDeviceMenu::onDeactivate));
    signal_selection_done().connect(sigc::mem_fun(*this, &VirtualBlockDeviceMenu::onSelectionDone));
    _menuChange.signal_activate().connect(sigc::mem_fun(*this, &VirtualBlockDeviceMenu::onChange));
    _menuDetach.signal_activate().connect(sigc::mem_fun(*this, &VirtualBlockDeviceMenu::onDetach));
    _menuChangeCd.signal_activate().connect(sigc::mem_fun(*this, &VirtualBlockDeviceMenu::onChangeCd));
}


void VirtualBlockDeviceMenu::popup(guint button, guint32 activateTime, const RefPtr<VirtualBlockDevice>& vbd)
{
    _vbd = vbd;
    if (!_vbd)
    {
        return;
    }
    XenPtr<xen_vbd_record> vbdRecord = _vbd->getRecord();
    if (vbdRecord->type == XEN_VBD_TYPE_DISK)
    {
        _menuChangeCd.hide();
    }
    else if (vbdRecord->type == XEN_VBD_TYPE_CD)
    {
        _menuChangeCd.show();
    }
    else
    {
        return;
    }
    RefPtr<VirtualMachine> vm = _vbd->getVm();
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


void VirtualBlockDeviceMenu::popup(guint button, guint32 activateTime, const Glib::ustring& name)
{
    _name = name;
    if (_name == "bootable")
    {
        _menuChange.set_sensitive(true);
    }
    else
    {
        _menuChange.set_sensitive(false);
    }
    Gtk::Menu::popup(button, activateTime);
}


void VirtualBlockDeviceMenu::onDeactivate()
{
    // This is called right after the closure of the popup window.
}


void VirtualBlockDeviceMenu::onSelectionDone()
{
    // This is called after all the things are done.
    if (_name.empty())
    {
        _vbd.reset();
    }
}


void VirtualBlockDeviceMenu::onChange()
{
    if (_name == "bootable")
    {
        //Controller::instance().toggleBootable(_vbd);
    }
}


void VirtualBlockDeviceMenu::onDetach()
{
    //Controller::instance().detachVbd(_vbd);
}


void VirtualBlockDeviceMenu::onChangeCd()
{
    //Controller::instance().changeCd2(_vbd);
}
