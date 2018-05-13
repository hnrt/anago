// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_VIRTUALBLOCKDEVICEMENU_H
#define HNRT_VIRTUALBLOCKDEVICEMENU_H


#include "Base/RefPtr.h"
#include "View/NameValueMenu.h"


namespace hnrt
{
    class VirtualBlockDevice;

    class VirtualBlockDeviceMenu
        : public NameValueMenu
    {
    public:

        VirtualBlockDeviceMenu();
        VirtualBlockDeviceMenu(const RefPtr<VirtualBlockDevice>&);
        void popup(guint, guint32, const RefPtr<VirtualBlockDevice>&);
        virtual void popup(guint, guint32, const Glib::ustring& name);

    protected:

        VirtualBlockDeviceMenu(const VirtualBlockDeviceMenu&);
        void operator =(const VirtualBlockDeviceMenu&);
        void init();
        void onDeactivate();
        void onSelectionDone();
        void onChange();
        void onDetach();
        void onChangeCd();

        Gtk::MenuItem _menuChange;
        Gtk::MenuItem _menuDetach;
        Gtk::MenuItem _menuChangeCd;
        Gtk::MenuItem _menuCancel;
        RefPtr<VirtualBlockDevice> _vbd;
        Glib::ustring _name;
    };
}


#endif //!HNRT_VIRTUALBLOCKDEVICEMENU_H
