// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_CDIMAGELISTVIEW_H
#define HNRT_CDIMAGELISTVIEW_H


#include <gtkmm.h>


namespace hnrt
{
    class Session;

    class CdImageListView
        : public Gtk::TreeView
    {
    public:

        CdImageListView(const Session&, bool);
        Glib::ustring getSelected() const;
        void select(const Glib::ustring&);
        Gtk::ScrolledWindow* createScrolledWindow();

    private:

        struct Record
            : public Gtk::TreeModel::ColumnRecord
        {
            Gtk::TreeModelColumn<Glib::ustring> colName;
            Gtk::TreeModelColumn<Glib::ustring> colValue;

            Record()
            {
                add(colName);
                add(colValue);
            }

        private:

            Record(const Record&);
            void operator =(const Record&);
        };

        CdImageListView(const CdImageListView&);
        void operator =(const CdImageListView&);
        void initStore(const Session&, bool);

        Record _record;
        Glib::RefPtr<Gtk::ListStore> _store;
    };
}


#endif //!HNRT_CDIMAGELISTVIEW_H
