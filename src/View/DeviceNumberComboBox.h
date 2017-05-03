// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_DEVICENUMBERCOMBOBOX_H
#define HNRT_DEVICENUMBERCOMBOBOX_H


#include <gtkmm.h>


namespace hnrt
{
    class DeviceNumberComboBox
        : public Gtk::ComboBox
    {
    public:

        DeviceNumberComboBox();
        virtual ~DeviceNumberComboBox();
        virtual Glib::ustring getSelected();

    protected:

        struct Record
            : public Gtk::TreeModel::ColumnRecord
        {
            Gtk::TreeModelColumn<Glib::ustring> colDisplayName;

            Record()
            {
                add(colDisplayName);
            }

        private:

            Record(const Record&);
            void operator =(const Record&);
        };

        DeviceNumberComboBox(const DeviceNumberComboBox&);
        void operator =(const DeviceNumberComboBox&);

        Record _record;
        Glib::RefPtr<Gtk::ListStore> _store;
    };
}


#endif //!HNRT_DEVICENUMBERCOMBOBOX_H
