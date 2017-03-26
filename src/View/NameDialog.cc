// Copyright (C) 2012-2017 Hideaki Narita


#include <string.h>
#include <libintl.h>
#include "NameDialog.h"


using namespace hnrt;


NameDialog::NameDialog(Gtk::Window& parent, const char* title)
    : Gtk::Dialog(title, parent)
    , _labelLabel(gettext("Label:"))
    , _descriptionLabel(gettext("Description:"))
{
    set_default_size(300, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();

    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _table.attach(_labelLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_labelEntry, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_descriptionLabel, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
    _table.attach(_descriptionEntry, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);

    _labelLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _descriptionLabel.set_alignment(1.0, 0.5); // h=right, v=center

    show_all_children();
}


Glib::ustring NameDialog::getLabel() const
{
    return _labelEntry.get_text();
}


void NameDialog::setLabel(const char* value)
{
    _labelEntry.set_text(value);
}


Glib::ustring NameDialog::getDescription() const
{
    return _descriptionEntry.get_text();
}


void NameDialog::setDescription(const char* value)
{
    _descriptionEntry.set_text(value);
}
