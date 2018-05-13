// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_VIRTUALINTERFACEMENU_H
#define HNRT_VIRTUALINTERFACEMENU_H


#include "Base/RefPtr.h"
#include "NameValueMenu.h"


namespace hnrt
{
    class VirtualInterface;

    class VirtualInterfaceMenu
        : public NameValueMenu
    {
    public:

        VirtualInterfaceMenu();
        VirtualInterfaceMenu(const RefPtr<VirtualInterface>&);
        void popup(guint, guint32, const RefPtr<VirtualInterface>&);
        virtual void popup(guint, guint32, const Glib::ustring& name);

    protected:

        VirtualInterfaceMenu(const VirtualInterfaceMenu&);
        void operator =(const VirtualInterfaceMenu&);
        void init();
        void onDeactivate();
        void onSelectionDone();
        void onChange();
        void onDetach();

        Gtk::MenuItem _menuChange;
        Gtk::MenuItem _menuDetach;
        Gtk::MenuItem _menuCancel;
        RefPtr<VirtualInterface> _vif;
        Glib::ustring _name;
    };
}


#endif //!HNRT_VIRTUALINTERFACEMENU_H
