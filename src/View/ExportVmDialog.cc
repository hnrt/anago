// Copyright (C) 2012-2017 Hideaki Narita


#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include "ExportVmDialog.h"


using namespace hnrt;


ExportVmDialog::ExportVmDialog(Gtk::Window& parent)
    : Gtk::Dialog(gettext("Virtual machine - Export"), parent)
    , _table(2, 3)
    , _filenameLabel(gettext("File name:"))
    , _browseButton(gettext("_Browse"))
    , _verifyButton(gettext("_Verify"))
{
    set_default_size(450, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    _pOkButton = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

    Gtk::VBox* box = get_vbox();

    _filenameLabel.set_use_underline(true);
    _browseButton.set_use_underline(true);
    _verifyButton.set_use_underline(true);

    _table.attach(_filenameLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_filenameEntry, 1, 2, 0, 1);
    _table.attach(_browseButton, 2, 3, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_verifyButton, 1, 3, 1, 2);
    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_SHRINK);

    show_all_children();

    _filenameEntry.signal_changed().connect(sigc::mem_fun(*this, &ExportVmDialog::onChange));
    _browseButton.signal_clicked().connect(sigc::mem_fun(*this, &ExportVmDialog::onBrowse));

    updateSensitivity();
}


Glib::ustring ExportVmDialog::getPath() const
{
    return _filenameEntry.get_text();
}


void ExportVmDialog::setPath(const Glib::ustring& path)
{
    _filenameEntry.set_text(path);
}


bool ExportVmDialog::getVerify() const
{
    return _verifyButton.get_active();
}


void ExportVmDialog::setVerify(bool value)
{
    _verifyButton.set_active(value);
}


void ExportVmDialog::onChange()
{
    updateSensitivity();
}


void ExportVmDialog::onBrowse()
{
    Gtk::FileChooserDialog dialog(*this, gettext("Choose file name to export"), Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    Glib::ustring path(_filenameEntry.get_text());
    Glib::ustring::size_type pos = path.rfind('/');
    if (pos != Glib::ustring::npos)
    {
        pos++;
        Glib::ustring folder = path.substr(0, pos);
        Glib::ustring name = path.substr(pos);
        dialog.set_current_folder(folder);
        dialog.set_current_name(name);
    }
    int response = dialog.run();
    if (response == Gtk::RESPONSE_OK)
    {
        _filenameEntry.set_text(dialog.get_filename());
    }
}


void ExportVmDialog::updateSensitivity()
{
    Glib::ustring path(_filenameEntry.get_text());
    _pOkButton->set_sensitive(!path.empty());
}
