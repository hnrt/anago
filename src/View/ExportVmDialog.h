// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_EXPORTVMDIALOG_H
#define HNRT_EXPORTVMDIALOG_H


#include <gtkmm.h>


namespace hnrt
{
    class ExportVmDialog
        : public Gtk::Dialog
    {
    public:

        ExportVmDialog(Gtk::Window&);
        Glib::ustring getPath() const;
        void setPath(const Glib::ustring&);
        bool getVerify() const;
        void setVerify(bool);

    private:

        ExportVmDialog(const ExportVmDialog&);
        void operator =(const ExportVmDialog&);
        void onChange();
        void onBrowse();
        void updateSensitivity();

        Gtk::Table _table;
        Gtk::Label _filenameLabel;
        Gtk::Entry _filenameEntry;
        Gtk::Button _browseButton;
        Gtk::CheckButton _verifyButton;
        Gtk::Button* _pOkButton;
    };
}


#endif //!HNRT_EXPORTVMDIALOG_H
