// Copyright (C) 2012-2017 Hideaki Narita


#include <string.h>
#include <libintl.h>
#include "Model/Model.h"
#include "Util/Util.h"
#include "CopyVmDialog.h"


using namespace hnrt;


CopyVmDialog::CopyVmDialog(Gtk::Window& parent, Session& session, const char* name, const char* srREFID)
    : Gtk::Dialog(gettext("Copy VM"), parent)
    , _nameLabel(gettext("Name:"))
    , _methodLabel(gettext("Method:"))
    , _cloneButton(_opGroup, gettext("Clone"))
    , _copyButton(_opGroup, gettext("Copy"))
    , _srLabel(gettext("Storage:"))
    , _srComboBox(session)
{
    set_default_size(300, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();

    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _table.attach(_nameLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_nameEntry, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_methodLabel, 0, 1, 1, 3, Gtk::FILL, Gtk::FILL);
    _table.attach(_cloneButton, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_copyButton, 1, 2, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_srLabel, 0, 1, 3, 4, Gtk::FILL, Gtk::FILL);
    _table.attach(_srComboBox, 1, 2, 3, 4, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);

    _nameLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _methodLabel.set_alignment(1.0, 0.0); // h=right, v=top
    _srLabel.set_alignment(1.0, 0.5); // h=right, v=center

    _nameEntry.set_text(Glib::ustring::compose(gettext("Copy of %1"), name));

    _copyButton.signal_toggled().connect(sigc::mem_fun(*this, &CopyVmDialog::onToggled));

    show_all_children();

    select(Glib::ustring(srREFID));
    onToggled();
}


void CopyVmDialog::onToggled()
{
    _srComboBox.set_sensitive(_copyButton.get_active());
}


void CopyVmDialog::select(const Glib::ustring& refid)
{
    _srComboBox.select(refid);
}


Glib::ustring CopyVmDialog::getName()
{
    return _nameEntry.get_text();
}


bool CopyVmDialog::isCopy()
{
    return _copyButton.get_active();
}


Glib::ustring CopyVmDialog::getSr()
{
    return _srComboBox.getSelected();
}
