// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_IMPORTVMDIALOG_H
#define HNRT_IMPORTVMDIALOG_H


#include <gtkmm.h>


namespace hnrt
{
    class ImportVmDialog
        : public Gtk::Dialog
    {
    public:

        ImportVmDialog(Gtk::Window&);
        Glib::ustring getPath() const;
        void setPath(const Glib::ustring&);

    private:

        ImportVmDialog(const ImportVmDialog&);
        void operator =(const ImportVmDialog&);
        void onChange();
        void onBrowse();
        void updateSensitivity();

        Gtk::Table _table;
        Gtk::Label _filenameLabel;
        Gtk::Entry _filenameEntry;
        Gtk::Button _browseButton;
        Gtk::Button* _pOkButton;
    };
}


#endif //!HNRT_IMPORTVMDIALOG_H
