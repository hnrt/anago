// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_STORAGEREPOSITORYMENU_H
#define HNRT_STORAGEREPOSITORYMENU_H


#include "NameValueMenu.h"


namespace hnrt
{
    class StorageRepository;

    class StorageRepositoryMenu
        : public NameValueMenu
    {
    public:

        StorageRepositoryMenu();
        void popup(guint, guint32, StorageRepository&);
        virtual void popup(guint, guint32, const Glib::ustring&);

    protected:

        StorageRepositoryMenu(const StorageRepositoryMenu&);
        void operator =(const StorageRepositoryMenu&);
        void onDeactivate();
        void onSelectionDone();

        Gtk::MenuItem _menuChange;
        Gtk::MenuItem _menuAddHdd;
        Gtk::MenuItem _menuDeleteCifs;
        Gtk::MenuItem _menuCancel;
        Gtk::Menu _submenuChange;
        Gtk::MenuItem _menuChangeName;
        Gtk::MenuItem _menuSetDefault;
    };
}


#endif //!HNRT_STORAGEREPOSITORYMENU_H
