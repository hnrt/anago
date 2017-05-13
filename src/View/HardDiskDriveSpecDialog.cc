// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "XenServer/StorageRepository.h"
#include "XenServer/Session.h"
#include "HardDiskDriveSpecDialog.h"


using namespace hnrt;


HardDiskDriveSpecDialog::HardDiskDriveSpecDialog(Gtk::Window& parent, const Session& session, const Glib::ustring& title)
    : Gtk::Dialog(title, parent)
    , _srLabel(gettext("Storage:"))
    , _srComboBox(session)
    , _sizeLabel(gettext("Size:"))
    , _labelLabel(gettext("Label:"))
    , _descLabel(gettext("Description:"))
{
    set_default_size(300, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();

    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _table.attach(_srLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_srComboBox, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_sizeLabel, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
    _table.attach(_sizeBox, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_labelLabel, 0, 1, 2, 3, Gtk::FILL, Gtk::FILL);
    _table.attach(_labelEntry, 1, 2, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_descLabel, 0, 1, 3, 4, Gtk::FILL, Gtk::FILL);
    _table.attach(_descEntry, 1, 2, 3, 4, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);

    _srLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _sizeLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _labelLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _descLabel.set_alignment(1.0, 0.5); // h=right, v=center

    show_all_children();
}


void HardDiskDriveSpecDialog::getValue(HardDiskDriveSpec& spec)
{
    spec.srREFID = _srComboBox.getSelected();
    spec.size = _sizeBox.getValue();
    spec.label = _labelEntry.get_text();
    spec.description = _descEntry.get_text();
}


void HardDiskDriveSpecDialog::setValue(const HardDiskDriveSpec& spec)
{
    _srComboBox.select(spec.srREFID);
    _sizeBox.setValue(spec.size);
    _labelEntry.set_text(spec.label);
    _descEntry.set_text(spec.description);
}
