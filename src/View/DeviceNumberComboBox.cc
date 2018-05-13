// Copyright (C) 2012-2018 Hideaki Narita


#include "DeviceNumberComboBox.h"


using namespace hnrt;


DeviceNumberComboBox::DeviceNumberComboBox()
{
    _store = Gtk::ListStore::create(_record);
    set_model(_store);
    pack_start(_record.colDisplayName, true);
}


DeviceNumberComboBox::~DeviceNumberComboBox()
{
}


Glib::ustring DeviceNumberComboBox::getSelected()
{
    Glib::ustring selected;
    Gtk::TreeIter iter = get_active();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        selected = row[_record.colDisplayName];
    }
    return selected;
}
