// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_HARDDISKDRIVELISTBOX_H
#define HNRT_HARDDISKDRIVELISTBOX_H


#include <gtkmm.h>
#include "HardDiskDriveListView.h"


namespace hnrt
{
    class Session;

    class HardDiskDriveListBox
        : public Gtk::HBox
    {
    public:

        HardDiskDriveListBox(Gtk::Window&, Session&);
        const HardDiskDriveListView& listView() const { return _listView; }
        HardDiskDriveListView& listView() { return _listView; }

    protected:

        HardDiskDriveListBox(const HardDiskDriveListBox&);
        void operator =(const HardDiskDriveListBox&);
        void onListViewChanged();
        void onSelectionChanged();
        void onAdd();
        void onEdit();
        void onRemove();

        Gtk::Window& _parent;
        HardDiskDriveListView _listView;
        Gtk::VBox _buttonsBox;
        Gtk::Button _addButton;
        Gtk::Button _editButton;
        Gtk::Button _removeButton;
        Session& _session;
    };
}


#endif //!HNRT_HARDDISKDRIVELISTBOX_H
