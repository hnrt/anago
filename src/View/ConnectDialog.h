// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONNECTDIALOG_H
#define HNRT_CONNECTDIALOG_H


#include <gtkmm.h>
#include "Model/ConnectSpec.h"


namespace hnrt
{
    class ConnectDialog
        : public Gtk::Dialog
    {
    public:

        ConnectDialog(Gtk::Window&, const char*, const Gtk::StockID&);
        void select(const ConnectSpec&);
        void selectFirstHostname();
        ConnectSpec getConnectSpec() const;
        Glib::ustring getDispname() const;
        Glib::ustring getHostname() const;
        Glib::ustring getUsername() const;
        Glib::ustring getPassword() const;

    private:

        struct CredentialsRecord
            : public Gtk::TreeModel::ColumnRecord
        {
            Gtk::TreeModelColumn<Glib::ustring> colId;
            Gtk::TreeModelColumn<Glib::ustring> colDispname;
            Gtk::TreeModelColumn<Glib::ustring> colHostname;
            Gtk::TreeModelColumn<Glib::ustring> colUsername;
            Gtk::TreeModelColumn<Glib::ustring> colPassword;
            Gtk::TreeModelColumn<long> colLastAccess;

            CredentialsRecord()
            {
                add(colId);
                add(colDispname);
                add(colHostname);
                add(colUsername);
                add(colPassword);
                add(colLastAccess);
            }
        };

        ConnectDialog(const ConnectDialog&);
        void operator =(const ConnectDialog&);
        void initStore();
        void add(const ConnectSpec&);
        void validate();
        void onDispnameChanged();
        void onHostnameChanged1(const Glib::ustring&, int*);
        void onHostnameChanged2(int, int);
        void onUsernameChanged1(const Glib::ustring&, int*);
        void onUsernameChanged2(int, int);

        Gtk::Table _table;
        Gtk::Label _dispnameLabel;
        Gtk::ComboBoxEntry _dispnameCombo;
        Gtk::Label _hostnameLabel;
        Gtk::Entry _hostnameEntry;
        Gtk::Label _usernameLabel;
        Gtk::Entry _usernameEntry;
        Gtk::Label _passwordLabel;
        Gtk::Entry _passwordEntry;

        CredentialsRecord _credRecord;
        Glib::RefPtr<Gtk::ListStore> _credStore;

        bool _editMode;
        Glib::ustring _uuid;
    };
}


#endif //!HNRT_CONNECTDIALOG_H
