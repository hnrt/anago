// Copyright (C) 2012-2018 Hideaki Narita


#include <libintl.h>
#include "XenServer/VirtualMachine.h"
#include "AttachNicDialog.h"


using namespace hnrt;


AttachNicDialog::AttachNicDialog(Gtk::Window& parent, const VirtualMachine& vm)
    : Gtk::Dialog(gettext("Attach NIC to VM"), parent)
    , _devLabel(gettext("Device:"))
    , _devCombo(vm)
    , _nwLabel(gettext("Network:"))
    , _nwLv(vm.getSession())
{
    set_default_size(400, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    _pApplyButton = add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();

    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _devBox.pack_start(_devCombo, Gtk::PACK_SHRINK);
    _devBox.set_spacing(6);

    _table.attach(_devLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_devBox, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_nwLabel, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
    _table.attach(*Gtk::manage(_nwLv.createScrolledWindow()), 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);

    _devLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _nwLabel.set_alignment(1.0, 0.0); // h=right, v=top

    _nwLv.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &AttachNicDialog::onNetworkChanged));

    show_all_children();

    _pApplyButton->set_sensitive(false);
}


AttachNicDialog::~AttachNicDialog()
{
}


void AttachNicDialog::onNetworkChanged()
{
    std::list<Glib::ustring> selected;
    _nwLv.getSelected(selected);
    if (selected.size() == 1)
    {
        _nwSelected = selected.front();
        _pApplyButton->set_sensitive(true);
    }
    else
    {
        _pApplyButton->set_sensitive(false);
    }
}


Glib::ustring AttachNicDialog::getDevice()
{
    return _devCombo.getSelected();
}


Glib::ustring AttachNicDialog::getNetwork()
{
    return _nwSelected;
}
