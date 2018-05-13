// Copyright (C) 2012-2018 Hideaki Narita


#include <libintl.h>
#include "XenServer/VirtualMachine.h"
#include "AttachCdDialog.h"


using namespace hnrt;


AttachCdDialog::AttachCdDialog(Gtk::Window& parent, const VirtualMachine& vm)
    : Gtk::Dialog(gettext("Attach CD drive to VM"), parent)
    , _devLabel(gettext("Device:"))
    , _devCombo(vm)
{
    set_default_size(-1, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();

    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _devBox.pack_start(_devCombo, Gtk::PACK_SHRINK);
    _devBox.set_spacing(6);

    _table.attach(_devLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_devBox, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);

    _devLabel.set_alignment(1.0, 0.5); // h=right, v=center

    show_all_children();
}


AttachCdDialog::~AttachCdDialog()
{
}


Glib::ustring AttachCdDialog::getUserdevice()
{
    return _devCombo.getSelected();
}
