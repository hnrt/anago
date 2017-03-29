// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_NETWORKMENU_H
#define HNRT_NETWORKMENU_H


#include "NameValueMenu.h"


namespace hnrt
{
    class Network;

    class NetworkMenu
        : public NameValueMenu
    {
    public:

        NetworkMenu(Network&);
        virtual void popup(guint, guint32, const Glib::ustring&);

    protected:

        NetworkMenu(const NetworkMenu&);
        void operator =(const NetworkMenu&);
        void onDeactivate();
        void onSelectionDone();
        void onChange();

        Network& _network;
        bool _readonly;
        Gtk::MenuItem _menuChange;
        Gtk::MenuItem _menuCancel;
        Glib::ustring _name;
    };
}


#endif //!HNRT_NETWORKMENU_H
