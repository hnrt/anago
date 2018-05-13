// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_CDDEVICECOMBOBOX_H
#define HNRT_CDDEVICECOMBOBOX_H


#include <gtkmm.h>


namespace hnrt
{
    class VirtualMachine;

    class CdDeviceComboBox
        : public Gtk::ComboBox
    {
    public:

        CdDeviceComboBox(const VirtualMachine&);
        void select(const Glib::ustring&);
        Glib::ustring getSelected() const;
        Glib::ustring getSelectedImage() const;

    private:

        struct Record
            : public Gtk::TreeModel::ColumnRecord
        {
            Gtk::TreeModelColumn<Glib::ustring> colREFID;
            Gtk::TreeModelColumn<Glib::ustring> colName;
            Gtk::TreeModelColumn<Glib::ustring> colImageREFID;

            Record()
            {
                add(colREFID);
                add(colName);
                add(colImageREFID);
            }
        };

        CdDeviceComboBox(const CdDeviceComboBox&);
        void operator =(const CdDeviceComboBox&);
        void initStore(const VirtualMachine&);

        Record _record;
        Glib::RefPtr<Gtk::ListStore> _store;
    };
}


#endif //!HNRT_CDDEVICECOMBOBOX_H
