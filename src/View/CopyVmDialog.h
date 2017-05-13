// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_COPYVMDIALOG_H
#define HNRT_COPYVMDIALOG_H


#include "StorageRepositoryComboBox.h"


namespace hnrt
{
    class CopyVmDialog
        : public Gtk::Dialog
    {
    public:

        CopyVmDialog(Gtk::Window&, const Session&, const char*, const char*);
        void select(const Glib::ustring&);
        Glib::ustring getName();
        bool isCopy();
        Glib::ustring getSr();

    private:

        CopyVmDialog(const CopyVmDialog&);
        void operator =(const CopyVmDialog&);
        void onToggled();

        Gtk::Table _table;
        Gtk::Label _nameLabel;
        Gtk::Entry _nameEntry;
        Gtk::Label _methodLabel;
        Gtk::RadioButtonGroup _opGroup;
        Gtk::RadioButton _cloneButton;
        Gtk::RadioButton _copyButton;
        Gtk::Label _srLabel;
        StorageRepositoryComboBox _srComboBox;
    };
}


#endif //!HNRT_COPYVMDIALOG_H
