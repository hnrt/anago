// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "VgaDialog.h"


using namespace hnrt;


VgaDialog::VgaDialog(Gtk::Window& parent)
    : Gtk::Dialog(gettext("Change video settings"), parent)
    , _vgaLabel(gettext("Adaptor:"))
    , _cirrusButton(_vgaGroup, gettext("Cirrus"))
    , _standardButton(_vgaGroup, gettext("Standard VGA"))
    , _videoramLabel(gettext("Video RAM:"))
    , _unitLabel(gettext("MB"))
{
    set_default_size(300, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();

    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _table.attach(_vgaLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_cirrusButton, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_standardButton, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_videoramLabel, 0, 1, 2, 3, Gtk::FILL, Gtk::FILL);
    _table.attach(_videoramBox, 1, 2, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);

    _vgaLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _cirrusButton.set_alignment(0.0, 0.5); // h=left, v=center
    _standardButton.set_alignment(0.0, 0.5); // h=left, v=center
    _videoramLabel.set_alignment(1.0, 0.5); // h=right, v=center
    
    _videoramBox.set_spacing(6);

    _videoramEntry.set_digits(0);
    _videoramEntry.set_increments(1.0, 4.0);
    _videoramEntry.set_range(1.0, 64.0);
    _videoramBox.pack_start(_videoramEntry, Gtk::PACK_SHRINK);

    _unitLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _videoramBox.pack_start(_unitLabel, Gtk::PACK_SHRINK);

    _standardButton.signal_toggled().connect(sigc::mem_fun(*this, &VgaDialog::onToggled));

    show_all_children();

    onToggled();
}


void VgaDialog::onToggled()
{
    _videoramEntry.set_sensitive(_standardButton.get_active());
}


bool VgaDialog::isStdVga()
{
    return _standardButton.get_active();
}


void VgaDialog::setStdVga(bool value)
{
    _cirrusButton.set_active(!value);
    _standardButton.set_active(value);
}


int VgaDialog::getVideoRam()
{
    return _videoramEntry.get_value_as_int();
}


void VgaDialog::setVideoRam(int value)
{
    _videoramEntry.set_value(value);
}
