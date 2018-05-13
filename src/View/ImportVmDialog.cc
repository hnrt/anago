// Copyright (C) 2012-2018 Hideaki Narita


#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include "Logger/Trace.h"
#include "ImportVmDialog.h"


using namespace hnrt;


ImportVmDialog::ImportVmDialog(Gtk::Window& parent)
    : Gtk::Dialog(gettext("Virtual machine - Import"), parent)
    , _table(1, 3)
    , _filenameLabel(gettext("File name:"))
    , _browseButton(gettext("_Browse"))
{
    set_default_size(450, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    _pOkButton = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

    Gtk::VBox* box = get_vbox();

    _filenameLabel.set_use_underline(true);
    _browseButton.set_use_underline(true);

    _table.attach(_filenameLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_filenameEntry, 1, 2, 0, 1);
    _table.attach(_browseButton, 2, 3, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_SHRINK);

    show_all_children();

    _filenameEntry.signal_changed().connect(sigc::mem_fun(*this, &ImportVmDialog::onChange));
    _browseButton.signal_clicked().connect(sigc::mem_fun(*this, &ImportVmDialog::onBrowse));

    updateSensitivity();
}


Glib::ustring ImportVmDialog::getPath() const
{
    return _filenameEntry.get_text();
}


void ImportVmDialog::setPath(const Glib::ustring& path)
{
    _filenameEntry.set_text(path);
}


void ImportVmDialog::onChange()
{
    updateSensitivity();
}


void ImportVmDialog::onBrowse()
{
    Gtk::FileChooserDialog dialog(*this, gettext("Choose file name to import"), Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    Glib::ustring path(_filenameEntry.get_text());
    if (path.empty())
    {
        dialog.set_current_folder(getenv("HOME"));
    }
    else
    {
        dialog.set_filename(path);
    }
    int response = dialog.run();
    if (response == Gtk::RESPONSE_OK)
    {
        _filenameEntry.set_text(dialog.get_filename());
    }
}


void ImportVmDialog::updateSensitivity()
{
    _pOkButton->set_sensitive(!_filenameEntry.get_text().empty());
}
