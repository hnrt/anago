// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Controller/Controller.h"
#include "Model/Model.h"
#include "XenServer/VirtualMachine.h"
#include "SnapshotMenu.h"


using namespace hnrt;


SnapshotMenu::SnapshotMenu()
    : _menuChangeName(gettext("Change label/description"))
    , _menuCreate(gettext("Create"))
    , _menuRevert(gettext("Revert"))
    , _menuDelete(gettext("Delete"))
    , _menuCancel(gettext("Cancel"))
{
    append(_menuChangeName);
    append(_menuCreate);
    append(_menuRevert);
    append(_menuDelete);
    append(*Gtk::manage(new Gtk::SeparatorMenuItem));
    append(_menuCancel);
    show_all_children();
    signal_deactivate().connect(sigc::mem_fun(*this, &SnapshotMenu::onDeactivate));
    signal_selection_done().connect(sigc::mem_fun(*this, &SnapshotMenu::onSelectionDone));
    _menuChangeName.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::changeSnapshotName));
    _menuCreate.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::snapshotVm));
    _menuRevert.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::revertVm));
    _menuDelete.signal_activate().connect(sigc::mem_fun(Controller::instance(), &Controller::deleteSnapshot));
}


void SnapshotMenu::popup(guint button, guint32 activateTime, const RefPtr<VirtualMachine>& vm)
{
    Model::instance().selectSnapshot(vm);
    XenPtr<xen_vm_record> record = vm->getRecord();
    if (record->is_a_snapshot)
    {
        _menuCreate.hide();
        _menuRevert.show();
        _menuDelete.show();
    }
    else
    {
        _menuCreate.show();
        _menuRevert.hide();
        _menuDelete.hide();
    }
    Gtk::Menu::popup(button, activateTime);
}


void SnapshotMenu::onDeactivate()
{
    // This is called right after the closure of the popup window.
}


void SnapshotMenu::onSelectionDone()
{
    // This is called after all the things are done.
}
