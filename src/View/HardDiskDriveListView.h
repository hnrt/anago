// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_HARDDISKDRIVELISTVIEW_H
#define HNRT_HARDDISKDRIVELISTVIEW_H


#include <gtkmm.h>


namespace hnrt
{
    class Session;
    struct HardDiskDriveSpec;

    class HardDiskDriveListView
        : public Gtk::TreeView
    {
    public:

        HardDiskDriveListView(const Session&);
        sigc::signal<void> signalChanged() { return _sigChanged; }
        void clear();
        void addValue(const HardDiskDriveSpec&);
        void removeValue(int);
        void getValue(int, HardDiskDriveSpec&) const;
        void setValue(int, const HardDiskDriveSpec&);
        int getCount() const;
        int getSelected();
        Gtk::ScrolledWindow* createScrolledWindow();

    protected:

        struct Record
            : public Gtk::TreeModel::ColumnRecord
        {
            Gtk::TreeModelColumn<Glib::ustring> colSr;
            Gtk::TreeModelColumn<Glib::ustring> colSize;
            Gtk::TreeModelColumn<Glib::ustring> colLabel;
            Gtk::TreeModelColumn<Glib::ustring> colDesc;
            Gtk::TreeModelColumn<HardDiskDriveSpec> colSpec;

            Record();
        };

        HardDiskDriveListView(const HardDiskDriveListView&);
        void operator =(const HardDiskDriveListView&);
        void setValue(Gtk::TreeIter&, const HardDiskDriveSpec&);
        void removeValue(Gtk::TreeIter&);

        Record _record;
        Glib::RefPtr<Gtk::ListStore> _store;
        sigc::signal<void> _sigChanged;
        const Session& _session;
    };
}


#endif //!HNRT_HARDDISKDRIVELISTVIEW_H
