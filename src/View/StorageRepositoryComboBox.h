// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_SRCOMBOBOX_H
#define HNRT_SRCOMBOBOX_H


#include <gtkmm.h>


namespace hnrt
{
    class Session;
    class StorageRepository;

    class StorageRepositoryComboBox
        : public Gtk::ComboBox
    {
    public:

        StorageRepositoryComboBox(const Session&);
        virtual ~StorageRepositoryComboBox();
        void select(const Glib::ustring&);
        void selectDefault();
        Glib::ustring getSelected();

    private:

        struct Record
            : public Gtk::TreeModel::ColumnRecord
        {
            Gtk::TreeModelColumn<Glib::ustring> colDisplayName;
            Gtk::TreeModelColumn<Glib::ustring> colName;
            Gtk::TreeModelColumn<Glib::ustring> colValue;

            Record()
            {
                add(colDisplayName);
                add(colName);
                add(colValue);
            }

        private:

            Record(const Record&);
            void operator =(const Record&);
        };

        StorageRepositoryComboBox(const StorageRepositoryComboBox&);
        void operator =(const StorageRepositoryComboBox&);
        void initStore();
        void add(StorageRepository&);

        Record _record;
        Glib::RefPtr<Gtk::ListStore> _store;
        const Session& _session;
    };
}


#endif //!HNRT_SRCOMBOBOX_H
