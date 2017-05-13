// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "ResizeDialog.h"


using namespace hnrt;


ResizeDialog::ResizeDialog(Gtk::Window& parent)
    : Gtk::Dialog(gettext("Resize virtual disk image"), parent)
    , _sizeLabel(gettext("Size:"))
{
    set_default_size(300, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();

    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _table.attach(_sizeLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_sizeBox, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
 
    _sizeLabel.set_alignment(1.0, 0.5); // h=right, v=center

    show_all_children();
}


int64_t ResizeDialog::getSize()
{
    return _sizeBox.getValue();
}


void ResizeDialog::setSize(int64_t value)
{
    _sizeBox.setValue(value);
}
