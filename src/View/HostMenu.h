// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_HOSTMENU_H
#define HNRT_HOSTMENU_H


#include "NameValueMenu.h"


namespace hnrt
{
    class Host;

    class HostMenu
        : public NameValueMenu
    {
    public:

        enum Type
        {
            DEFAULT,
            NAME_VALUE,
        };

        HostMenu(Type = DEFAULT);
        virtual ~HostMenu();
        void popup(guint, guint32, Host&);
        virtual void popup(guint, guint32, const Glib::ustring&);

    protected:

        HostMenu(const HostMenu&);
        void operator =(const HostMenu&);
        void onDeactivate();
        void onSelectionDone();
        void onChange();

        Gtk::MenuItem _menuConnect;
        Gtk::MenuItem _menuDisconnect;
        Gtk::MenuItem _menuWake;
        Gtk::MenuItem _menuShutdown;
        Gtk::MenuItem _menuRestart;
        Gtk::MenuItem _menuChangeName;
        Gtk::MenuItem _menuAddVm;
        Gtk::MenuItem _menuAddCifs;
        Gtk::MenuItem _menuEditConnectSpec;
        Gtk::MenuItem _menuRemove;
        Gtk::MenuItem _menuChange;
        Gtk::Menu _submenuChange;
        Gtk::MenuItem _menuCancel;
        Glib::ustring _name;
    };
}


#endif //!HNRT_HOSTMENU_H
