// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_ADDCIFSDIALOG_H
#define HNRT_ADDCIFSDIALOG_H


#include <gtkmm.h>


namespace hnrt
{
    struct CifsSpec;

    class AddCifsDialog
        : public Gtk::Dialog
    {
    public:

        AddCifsDialog(Gtk::Window&);
        void getSpec(CifsSpec&);
        void setSpec(const CifsSpec&);

    private:

        AddCifsDialog(const AddCifsDialog&);
        void operator =(const AddCifsDialog&);
        void onLocationChanged1(const Glib::ustring&, int*);
        void onLocationChanged2(int, int);
        void onUsernameChanged1(const Glib::ustring&, int*);
        void onUsernameChanged2(int, int);
        void validateLocation();
        void validateUsername();
        void validate();

        Gtk::Table _table;
        Gtk::Label _labelLabel;
        Gtk::Entry _labelEntry;
        Gtk::Label _descriptionLabel;
        Gtk::Entry _descriptionEntry;
        Gtk::Label _locationLabel;
        Gtk::Entry _locationEntry;
        Gtk::Label _usernameLabel;
        Gtk::Entry _usernameEntry;
        Gtk::Label _passwordLabel;
        Gtk::Entry _passwordEntry;
        bool _locationValid;
        bool _usernameValid;
    };
}


#endif //!HNRT_ADDCIFSDIALOG_H
