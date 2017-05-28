// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_NETWORKMENU_H
#define HNRT_NETWORKMENU_H


#include "Base/RefPtr.h"
#include "NameValueMenu.h"


namespace hnrt
{
    class Network;

    class NetworkMenu
        : public NameValueMenu
    {
    public:

        NetworkMenu(const RefPtr<Network>&);
        NetworkMenu();
        virtual void popup(guint, guint32, const Glib::ustring&);
        void popup(guint button, guint32 activateTime, Network&);

    protected:

        NetworkMenu(const NetworkMenu&);
        void operator =(const NetworkMenu&);
        void init();
        void onDeactivate();
        void onSelectionDone();
        void onChange();

        Gtk::MenuItem _menuChange;
        Gtk::MenuItem _menuCancel;
        RefPtr<Network> _network;
        bool _readonly;
        Glib::ustring _name;
    };
}


#endif //!HNRT_NETWORKMENU_H
