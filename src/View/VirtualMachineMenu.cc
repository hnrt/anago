// Copyright (C) 2012-2018 Hideaki Narita


#include <libintl.h>
#include "Controller/Controller.h"
#include "XenServer/VirtualMachine.h"
#include "VirtualMachineMenu.h"


using namespace hnrt;


VirtualMachineMenu::VirtualMachineMenu()
    : _menuStart(gettext("Start"))
    , _menuShutdown(gettext("Shut down"))
    , _menuReboot(gettext("Reboot"))
    , _menuSuspend(gettext("Suspend"))
    , _menuResume(gettext("Resume"))
    , _menuChangeCd(gettext("Change CD/DVD"))
    , _menuSendSas(gettext("Send CTRL+ALT+DEL"))
    , _menuChange(gettext("Change"))
    , _menuCopy(gettext("Copy"))
    , _menuDelete(gettext("Delete"))
    , _menuExport(gettext("Export"))
    , _menuCancel(gettext("Cancel"))
    , _menuChangeName(gettext("Label/Description"))
    , _menuChangeCpu(gettext("CPU"))
    , _menuChangeMemory(gettext("Memory"))
    , _menuChangeShadowMemory(gettext("Shadow memory"))
    , _menuChangeVga(gettext("VGA"))
    , _menuAttachHdd(gettext("Attach HDD"))
    , _menuAttachCd(gettext("Attach CD drive"))
    , _menuAttachNic(gettext("Attach NIC"))
{
    append(_menuStart);
    append(_menuShutdown);
    append(_menuReboot);
    append(_menuSuspend);
    append(_menuResume);
    append(_menuChangeCd);
    append(_menuSendSas);
    append(_menuChange);
    append(_menuCopy);
    append(_menuDelete);
    append(_menuExport);
    append(*Gtk::manage(new Gtk::SeparatorMenuItem));
    append(_menuCancel);
    _submenuChange.append(_menuChangeName);
    _submenuChange.append(_menuChangeCpu);
    _submenuChange.append(_menuChangeMemory);
    _submenuChange.append(_menuChangeShadowMemory);
    _submenuChange.append(_menuChangeVga);
    _submenuChange.append(_menuAttachHdd);
    _submenuChange.append(_menuAttachCd);
    _submenuChange.append(_menuAttachNic);
    _menuChange.set_submenu(_submenuChange);
    show_all_children();
    signal_deactivate().connect(sigc::mem_fun(*this, &VirtualMachineMenu::onDeactivate));
    signal_selection_done().connect(sigc::mem_fun(*this, &VirtualMachineMenu::onSelectionDone));
    _menuStart.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::startVm));
    _menuShutdown.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::shutdownVm));
    _menuReboot.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::rebootVm));
    _menuSuspend.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::suspendVm));
    _menuResume.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::resumeVm));
    _menuChangeCd.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::changeCd));
    _menuSendSas.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::sendCtrlAltDelete));
    _menuCopy.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::copyVm));
    _menuDelete.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::deleteVm));
    _menuExport.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::exportVm));
    _menuChangeName.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::changeVmName));
    _menuChangeCpu.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::changeCpu));
    _menuChangeMemory.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::changeMemory));
    _menuChangeShadowMemory.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::changeShadowMemory));
    _menuChangeVga.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::changeVga));
    _menuAttachHdd.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::attachHdd));
    _menuAttachCd.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::attachCd));
    _menuAttachNic.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::attachNic));
}


void VirtualMachineMenu::popup(guint button, guint32 activateTime, VirtualMachine& vm)
{
    XenPtr<xen_vm_record> record = vm.getRecord();
    bool sensitive = vm.isBusy() ? false : true;
    switch (record->power_state)
    {
    case XEN_VM_POWER_STATE_HALTED:
    default:
        _menuStart.show();
        _menuShutdown.hide();
        _menuReboot.hide();
        _menuSuspend.hide();
        _menuResume.hide();
        _menuChangeCd.show();
        _menuSendSas.hide();
        _menuChange.show();
        _menuCopy.show();
        _menuDelete.show();
        _menuExport.show();
        _menuStart.set_sensitive(sensitive);
        _menuChangeCd.set_sensitive(sensitive);
        _menuChange.set_sensitive(sensitive);
        _menuCopy.set_sensitive(sensitive);
        _menuDelete.set_sensitive(sensitive);
        _menuExport.set_sensitive(sensitive);
        break;
    case XEN_VM_POWER_STATE_PAUSED:
        _menuStart.hide();
        _menuShutdown.hide();
        _menuReboot.hide();
        _menuSuspend.hide();
        _menuResume.show();
        _menuChangeCd.hide();
        _menuSendSas.hide();
        _menuChange.hide();
        _menuCopy.hide();
        _menuDelete.hide();
        _menuExport.hide();
        _menuResume.set_sensitive(sensitive);
        break;
    case XEN_VM_POWER_STATE_RUNNING:
        _menuStart.hide();
        _menuShutdown.show();
        _menuReboot.show();
        _menuSuspend.show();
        _menuResume.hide();
        _menuChangeCd.show();
        _menuSendSas.show();
        _menuChange.hide();
        _menuCopy.hide();
        _menuDelete.hide();
        _menuExport.hide();
        _menuShutdown.set_sensitive(sensitive);
        _menuReboot.set_sensitive(sensitive);
        _menuSuspend.set_sensitive(sensitive);
        _menuChangeCd.set_sensitive(sensitive);
        _menuSendSas.set_sensitive(sensitive);
        break;
    case XEN_VM_POWER_STATE_SUSPENDED:
        _menuStart.hide();
        _menuShutdown.hide();
        _menuReboot.hide();
        _menuSuspend.hide();
        _menuResume.show();
        _menuChangeCd.hide();
        _menuSendSas.hide();
        _menuChange.hide();
        _menuCopy.hide();
        _menuDelete.hide();
        _menuExport.hide();
        _menuResume.set_sensitive(sensitive);
        break;
    }
    Gtk::Menu::popup(button, activateTime);
}


void VirtualMachineMenu::onDeactivate()
{
    // This is called right after the closure of the popup window.
}


void VirtualMachineMenu::onSelectionDone()
{
    // This is called after all the things are done.
}
