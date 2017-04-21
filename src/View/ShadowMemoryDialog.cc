// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "ShadowMemoryDialog.h"


using namespace hnrt;


ShadowMemoryDialog::ShadowMemoryDialog(Gtk::Window& parent)
    : Gtk::Dialog(gettext("Change shadow memory settings"), parent)
    , _mulLabel(gettext("Shadow memory multiplier:"))
{
    set_default_size(330, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();

    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _table.attach(_mulLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_mulSpinButton, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

    _mulLabel.set_alignment(1.0, 0.5); // h=right, v=center

    _mulSpinButton.set_digits(1);
    _mulSpinButton.set_increments(0.1, 1.0);
    _mulSpinButton.set_range(1.0, 100.0);
    _mulSpinButton.set_value(1.0);
    _mulSpinButton.set_tooltip_text(gettext("Typical value: 1.0 for general use, 4.0 for XenApp"));

    show_all_children();
}


double ShadowMemoryDialog::getMultiplier()
{
    return _mulSpinButton.get_value();
}


void ShadowMemoryDialog::setMultiplier(double value)
{
    _mulSpinButton.set_value(value);
}
