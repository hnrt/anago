// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_ATTACHCDDIALOG_H
#define HNRT_ATTACHCDDIALOG_H


#include "BlockDeviceNumberComboBox.h"


namespace hnrt
{
    class VirtualMachine;

    class AttachCdDialog
        : public Gtk::Dialog
    {
    public:

        AttachCdDialog(Gtk::Window&, const VirtualMachine&);
        virtual ~AttachCdDialog();
        Glib::ustring getUserdevice();

    private:


        AttachCdDialog(const AttachCdDialog&);
        void operator =(const AttachCdDialog&);

        Gtk::Table _table;
        Gtk::Label _devLabel;
        BlockDeviceNumberComboBox _devCombo;
        Gtk::HBox _devBox;
    };
}


#endif //!HNRT_ATTACHCDDIALOG_H
