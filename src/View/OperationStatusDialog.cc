// Copyright (C) 2017 Hideaki Narita


#include "OperationStatusDialog.h"


using namespace hnrt;


OperationStatusDialog::OperationStatusDialog(Gtk::Window& parent, const char* title)
    : Gtk::Dialog(title, parent)
    , _flags(0)
{
    set_default_size(-1, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_OK);

    Gtk::VBox* box = get_vbox();

    _statusBox.pack_start(_statusImage, Gtk::PACK_SHRINK);
    _statusBox.pack_start(_statusLabel, Gtk::PACK_EXPAND_WIDGET);
    box->pack_start(_statusBox, Gtk::PACK_EXPAND_WIDGET);

    show_all_children();

    _dispatcher.connect(sigc::mem_fun(*this, &OperationStatusDialog::onDispatch));
}


void OperationStatusDialog::setStatus(const char* status)
{
    {
        Glib::Mutex::Lock lock(_mutex);
        _status = status;
        _flags |= STATUS_CHANGED;
    }
    _dispatcher();
}


void OperationStatusDialog::onDispatch()
{
    Glib::Mutex::Lock lock(_mutex);
    if ((_flags & STATUS_CHANGED))
    {
        _statusLabel.set_text(_status);
    }
    _flags = 0;
}
