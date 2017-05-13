// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_SNAPSHOTTREEVIEW_H
#define HNRT_SNAPSHOTTREEVIEW_H


#include <gtkmm.h>
#include "Base/RefPtr.h"
#include "SnapshotMenu.h"


namespace hnrt
{
    class VirtualMachine;

    class SnapshotTreeView
        : public Gtk::TreeView
    {
    public:

        SnapshotTreeView();
        void clear();
        void set(const RefPtr<VirtualMachine>&);
        RefPtr<VirtualMachine> getSelected();

    protected:

        struct Record
            : public Gtk::TreeModel::ColumnRecord
        {
            Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > colPix;
            Gtk::TreeModelColumn<Glib::ustring> colName;
            Gtk::TreeModelColumn<Glib::ustring> colREFID;
            Gtk::TreeModelColumn<unsigned long> colTime;

            Record()
            {
                add(colPix);
                add(colName);
                add(colREFID);
                add(colTime);
            }

        private:

            Record(const Record&);
            void operator =(const Record&);
        };

        SnapshotTreeView(const SnapshotTreeView&);
        void operator =(const SnapshotTreeView&);
        void add(const RefPtr<VirtualMachine>&);
        Gtk::TreeIter find(const Glib::ustring&, Gtk::TreeIter);
        Gtk::TreeIter findPositionOfInsertion(const RefPtr<VirtualMachine>&, Gtk::TreeIter);
        void set(const Gtk::TreeModel::Row&, const RefPtr<VirtualMachine>&);
        virtual bool on_button_press_event(GdkEventButton*);

        Record _record;
        Glib::RefPtr<Gtk::TreeStore> _store;
        RefPtr<VirtualMachine> _vm;
        SnapshotMenu _menu;
    };
}


#endif //!HNRT_SNAPSHOTTREEVIEW_H
