// Copyright (C) 2012-2018 Hideaki Narita


#include <string.h>
#include <libintl.h>
#include "Util/Base64.h"
#include "Util/Scrambler.h"
#include "XenServer/CifsSpec.h"
#include "AddCifsDialog.h"


using namespace hnrt;


AddCifsDialog::AddCifsDialog(Gtk::Window& parent)
    : Gtk::Dialog(gettext("Add CIFS repository"), parent)
    , _labelLabel(gettext("Label:"))
    , _descriptionLabel(gettext("Description:"))
    , _locationLabel(gettext("Location:"))
    , _usernameLabel(gettext("User name:"))
    , _passwordLabel(gettext("Password:"))
    , _locationValid(false)
    , _usernameValid(false)
{
    set_default_size(300, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();
    box->set_spacing(6);

    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _table.attach(_labelLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_labelEntry, 1, 2, 0, 1);
    _table.attach(_descriptionLabel, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
    _table.attach(_descriptionEntry, 1, 2, 1, 2);
    _table.attach(_locationLabel, 0, 1, 2, 3, Gtk::FILL, Gtk::FILL);
    _table.attach(_locationEntry, 1, 2, 2, 3);
    _table.attach(_usernameLabel, 0, 1, 3, 4, Gtk::FILL, Gtk::FILL);
    _table.attach(_usernameEntry, 1, 2, 3, 4);
    _table.attach(_passwordLabel, 0, 1, 4, 5, Gtk::FILL, Gtk::FILL);
    _table.attach(_passwordEntry, 1, 2, 4, 5);

    _labelLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _descriptionLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _locationLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _usernameLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _passwordLabel.set_alignment(1.0, 0.5); // h=right, v=center

    _locationEntry.signal_insert_text().connect(sigc::mem_fun(*this, &AddCifsDialog::onLocationChanged1));
    _locationEntry.signal_delete_text().connect(sigc::mem_fun(*this, &AddCifsDialog::onLocationChanged2));
    _usernameEntry.signal_insert_text().connect(sigc::mem_fun(*this, &AddCifsDialog::onUsernameChanged1));
    _usernameEntry.signal_delete_text().connect(sigc::mem_fun(*this, &AddCifsDialog::onUsernameChanged2));

    _locationEntry.set_tooltip_text(gettext("e.g. \\\\server\\share"));

    _passwordEntry.set_visibility(false);
    _passwordEntry.set_invisible_char('*');

    show_all_children();

    validateLocation();
    validateUsername();
    validate();
}


void AddCifsDialog::onLocationChanged1(const Glib::ustring& text, int* position)
{
    validateLocation();
    validate();
}


void AddCifsDialog::onLocationChanged2(int start, int end)
{
    validateLocation();
    validate();
}


void AddCifsDialog::validateLocation()
{
    const char* s = _locationEntry.get_text().c_str();
    _locationValid = false;
    if (*s == '\\') s++; else return;
    if (*s == '\\') s++; else return;
    if (*s != '\0' && *s != '\\') s++; else return;
    while (*s != '\0' && *s != '\\') s++;
    if (*s == '\\') s++; else return;
    if (*s != '\0' && *s != '\\') s++; else return;
    _locationValid = true;
}


void AddCifsDialog::onUsernameChanged1(const Glib::ustring& text, int* position)
{
    validateUsername();
    validate();
}


void AddCifsDialog::onUsernameChanged2(int start, int end)
{
    validateUsername();
    validate();
}


void AddCifsDialog::validateUsername()
{
    _usernameValid = !_usernameEntry.get_text().empty();
}


void AddCifsDialog::validate()
{
    set_response_sensitive(Gtk::RESPONSE_APPLY, _locationValid && _usernameValid);
}


void AddCifsDialog::getSpec(CifsSpec& spec)
{
    spec.label = _labelEntry.get_text();

    spec.description = _descriptionEntry.get_text();

    spec.location = _locationEntry.get_text();

    spec.username = _usernameEntry.get_text();

    Glib::ustring password = _passwordEntry.get_text();
    Scrambler e1(password.data(), password.bytes());
    Base64Encoder e2(e1.getValue(), e1.getLength());
    spec.password = e2.getValue();
}


void AddCifsDialog::setSpec(const CifsSpec& spec)
{
    _labelEntry.set_text(spec.label);

    _descriptionEntry.set_text(spec.description);

    _locationEntry.set_text(spec.location);

    _usernameEntry.set_text(spec.username);

    Base64Decoder d1(spec.password.c_str());
    Descrambler d2(d1.getValue(), d1.getLength());
    _passwordEntry.set_text(reinterpret_cast<const char*>(d2.getValue()));
}
