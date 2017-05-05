// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_HARDDISKDRIVESPECDIALOG_H
#define HNRT_HARDDISKDRIVESPECDIALOG_H


#include "XenServer/HardDiskDriveSpec.h"
#include "SizeInBytesBox.h"
#include "StorageRepositoryComboBox.h"


namespace hnrt
{
    class Session;

    class HardDiskDriveSpecDialog
        : public Gtk::Dialog
    {
    public:

        HardDiskDriveSpecDialog(Gtk::Window&, const Session&, const Glib::ustring&);
        void getValue(HardDiskDriveSpec&);
        void setValue(const HardDiskDriveSpec&);

    private:

        HardDiskDriveSpecDialog(const HardDiskDriveSpecDialog&);
        void operator =(const HardDiskDriveSpecDialog&);

        Gtk::Table _table;
        Gtk::Label _srLabel;
        StorageRepositoryComboBox _srComboBox;
        Gtk::Label _sizeLabel;
        SizeInBytesBox _sizeBox;
        Gtk::Label _labelLabel;
        Gtk::Entry _labelEntry;
        Gtk::Label _descLabel;
        Gtk::Entry _descEntry;
    };
}


#endif //!HNRT_HARDDISKDRIVESPECDIALOG_H
