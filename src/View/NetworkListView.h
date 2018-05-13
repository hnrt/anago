// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_NETWORKLISTVIEW_H
#define HNRT_NETWORKLISTVIEW_H


#include <list>
#include <gtkmm.h>


namespace hnrt
{
    class Session;

    class NetworkListView
        : public Gtk::TreeView
    {
    public:

        NetworkListView(const Session& session);
        int getSelected(std::list<Glib::ustring>& list) const;
        Glib::SignalProxy0<void> signalSelectionChanged() { return get_selection()->signal_changed(); }
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

        NetworkListView(const NetworkListView&);
        void operator =(const NetworkListView&);
        void initStore(const Session& session);

        Record _record;
        Glib::RefPtr<Gtk::ListStore> _store;
    };
}


#endif //!HNRT_NETWORKLISTVIEW_H
