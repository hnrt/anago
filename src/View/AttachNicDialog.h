// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_ATTACHNICDIALOG_H
#define HNRT_ATTACHNICDIALOG_H


#include "NetworkDeviceNumberComboBox.h"
#include "NetworkListView.h"


namespace hnrt
{
    class VirtualMachine;

    class AttachNicDialog
        : public Gtk::Dialog
    {
    public:

        AttachNicDialog(Gtk::Window&, const VirtualMachine&);
        virtual ~AttachNicDialog();
        Glib::ustring getDevice();
        Glib::ustring getNetwork();

    private:


        AttachNicDialog(const AttachNicDialog&);
        void operator =(const AttachNicDialog&);
        void onNetworkChanged();

        Gtk::Table _table;
        Gtk::Label _devLabel;
        NetworkDeviceNumberComboBox _devCombo;
        Gtk::HBox _devBox;
        Gtk::Label _nwLabel;
        NetworkListView _nwLv;
        Gtk::Button* _pApplyButton;
        Glib::ustring _nwSelected;
    };
}


#endif //!HNRT_ATTACHNICDIALOG_H
