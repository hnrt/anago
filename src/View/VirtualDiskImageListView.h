// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALDISKIMAGELISTVIEW_H
#define HNRT_VIRTUALDISKIMAGELISTVIEW_H


#include "VirtualDiskImageMenu.h"


struct xen_vdi_record_opt_set;


namespace hnrt
{
    class Session;

    class VirtualDiskImageListView
        : public Gtk::TreeView
    {
    public:

        enum Constants
        {
            ATTACHABLE_ONLY = (1 << 0),
        };

        VirtualDiskImageListView();
        void update(Session&, const xen_vdi_record_opt_set*, int = 0);
        Glib::ustring getSelected();
        Gtk::ScrolledWindow* createScrolledWindow();

    protected:

        struct Record
            : public Gtk::TreeModel::ColumnRecord
        {
            Gtk::TreeModelColumn<Glib::ustring> colREFID;
            Gtk::TreeModelColumn<Glib::ustring> colUUID;
            Gtk::TreeModelColumn<Glib::ustring> colName;
            Gtk::TreeModelColumn<Glib::ustring> colDesc;
            Gtk::TreeModelColumn<Glib::ustring> colType;
            Gtk::TreeModelColumn<Glib::ustring> colSize;
            Gtk::TreeModelColumn<Glib::ustring> colUsed;
            Gtk::TreeModelColumn<Glib::ustring> colLocation;
            Gtk::TreeModelColumn<Glib::ustring> colSharable;
            Gtk::TreeModelColumn<Glib::ustring> colReadOnly;
            Gtk::TreeModelColumn<Glib::ustring> colManaged;
            Gtk::TreeModelColumn<Glib::ustring> colMissing;
            Gtk::TreeModelColumn<Glib::ustring> colSnapshot;
            Gtk::TreeModelColumn<Glib::ustring> colVm;
            Gtk::TreeModelColumn<Glib::ustring> colParent;
            Gtk::TreeModelColumn<bool> colUpdated;

            Record()
            {
                add(colREFID);
                add(colUUID);
                add(colName);
                add(colDesc);
                add(colType);
                add(colSize);
                add(colUsed);
                add(colLocation);
                add(colSharable);
                add(colReadOnly);
                add(colManaged);
                add(colMissing);
                add(colSnapshot);
                add(colVm);
                add(colParent);
                add(colUpdated);
            }

        private:

            Record(const Record&);
            void operator =(const Record&);
        };

        VirtualDiskImageListView(const VirtualDiskImageListView&);
        void operator =(const VirtualDiskImageListView&);
        virtual bool on_button_press_event(GdkEventButton*);

        Record _record;
        Glib::RefPtr<Gtk::ListStore> _store;
        VirtualDiskImageMenu _menu;
        Session* _pSession;
    };
}


#endif //!HNRT_VIRTUALDISKIMAGELISTVIEW_H
