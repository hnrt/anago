// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Controller/Controller.h"
#include "XenServer/VirtualBlockDevice.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/VirtualMachine.h"
#include "VirtualDiskImageMenu.h"


using namespace hnrt;


VirtualDiskImageMenu::VirtualDiskImageMenu()
    : _menuChange(gettext("Change"))
    , _menuChangeName(gettext("Change label/description"))
    , _menuResize(gettext("Resize"))
    , _menuRemove(gettext("Remove"))
    , _menuCancel(gettext("Cancel"))
{
    append(_menuChangeName);
    append(_menuResize);
    append(_menuRemove);
    init();
}


VirtualDiskImageMenu::VirtualDiskImageMenu(const RefPtr<VirtualDiskImage>& vdi)
    : _menuChange(gettext("Change"))
    , _menuCancel(gettext("Cancel"))
    , _vdi(vdi)
{
    append(_menuChange);
    init();
}


void VirtualDiskImageMenu::init()
{
    append(*Gtk::manage(new Gtk::SeparatorMenuItem));
    append(_menuCancel);
    show_all_children();
    signal_deactivate().connect(sigc::mem_fun(*this, &VirtualDiskImageMenu::onDeactivate));
    signal_selection_done().connect(sigc::mem_fun(*this, &VirtualDiskImageMenu::onSelectionDone));
    _menuChange.signal_activate().connect(sigc::mem_fun(*this, &VirtualDiskImageMenu::onChange));
    _menuChangeName.signal_activate().connect(sigc::mem_fun(*this, &VirtualDiskImageMenu::onChangeName));
    _menuResize.signal_activate().connect(sigc::mem_fun(*this, &VirtualDiskImageMenu::onResize));
    _menuRemove.signal_activate().connect(sigc::mem_fun(*this, &VirtualDiskImageMenu::onRemove));
}


void VirtualDiskImageMenu::popup(guint button, guint32 activateTime, const RefPtr<VirtualDiskImage>& vdi)
{
    _vdi = vdi;
    if (!_vdi)
    {
        return;
    }
    _menuRemove.show();
    XenPtr<xen_vdi_record> vdiRecord = _vdi->getRecord();
    if (vdiRecord->read_only)
    {
        _menuChangeName.set_sensitive(false);
        _menuResize.set_sensitive(false);
        _menuRemove.set_sensitive(false);
    }
    else if (vdiRecord->vbds && vdiRecord->vbds->size)
    {
        _menuChangeName.set_sensitive(true);
        _menuResize.set_sensitive(true);
        _menuRemove.set_sensitive(false);
    }
    else
    {
        _menuChangeName.set_sensitive(true);
        _menuResize.set_sensitive(true);
        _menuRemove.set_sensitive(true);
    }
    Gtk::Menu::popup(button, activateTime);
}


void VirtualDiskImageMenu::popup(guint button, guint32 activateTime, const Glib::ustring& name)
{
    _name = name;

    bool sensitive = false;
    if (_name == "Label" ||
        _name == "Description" ||
        _name == "virtual-size")
    {
        XenPtr<xen_vdi_record> vdiRecord = _vdi->getRecord();
        if (!vdiRecord->read_only)
        {
            sensitive = true;
        }
    }
    else
    {
        _name = "?";
    }
    _menuChange.set_sensitive(sensitive);
    Gtk::Menu::popup(button, activateTime);
}


void VirtualDiskImageMenu::onDeactivate()
{
    // This is called right after the closure of the popup window.
}


void VirtualDiskImageMenu::onSelectionDone()
{
    // This is called after all the things are done.
    if (_name.empty())
    {
        _vdi.reset();
    }
}


void VirtualDiskImageMenu::onChange()
{
    if (_name == "Label" ||
        _name == "Description")
    {
        Controller::instance().changeVdiName(*_vdi);
    }
    else if (_name == "virtual-size")
    {
        Controller::instance().resizeVdi(*_vdi);
    }
}


void VirtualDiskImageMenu::onChangeName()
{
    Controller::instance().changeVdiName(*_vdi);
}


void VirtualDiskImageMenu::onResize()
{
    Controller::instance().resizeVdi(*_vdi);
}


void VirtualDiskImageMenu::onRemove()
{
    Controller::instance().removeVdi(*_vdi);
}
