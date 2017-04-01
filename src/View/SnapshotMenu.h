// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_SNAPSHOTMENU_H
#define HNRT_SNAPSHOTMENU_H


#include <gtkmm.h>
#include "Base/RefPtr.h"


namespace hnrt
{
    class VirtualMachine;

    class SnapshotMenu
        : public Gtk::Menu
    {
    public:

        SnapshotMenu();
        void popup(guint, guint32, const RefPtr<VirtualMachine>&);

    protected:

        SnapshotMenu(const SnapshotMenu&);
        void operator =(const SnapshotMenu&);
        void onDeactivate();
        void onSelectionDone();

        Gtk::MenuItem _menuChangeName;
        Gtk::MenuItem _menuCreate;
        Gtk::MenuItem _menuRevert;
        Gtk::MenuItem _menuDelete;
        Gtk::MenuItem _menuCancel;
    };
}


#endif //!HNRT_SNAPSHOTMENU_H
