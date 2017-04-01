// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEGENLVMENU_H
#define HNRT_VIRTUALMACHINEGENLVMENU_H


#include "Base/RefPtr.h"
#include "NameValueMenu.h"


namespace hnrt
{
    class VirtualMachine;

    class VirtualMachineGenMenu
        : public NameValueMenu
    {
    public:

        VirtualMachineGenMenu(const RefPtr<VirtualMachine>&);
        virtual void popup(guint, guint32);
        virtual void popup(guint, guint32, const Glib::ustring&);

    protected:

        VirtualMachineGenMenu(const VirtualMachineGenMenu&);
        void operator =(const VirtualMachineGenMenu&);
        void onDeactivate();
        void onSelectionDone();
        void onChange();

        Gtk::MenuItem _menuChange;
        Gtk::MenuItem _menuChangeName;
        Gtk::MenuItem _menuChangeCpu;
        Gtk::MenuItem _menuChangeShadowMemory;
        Gtk::MenuItem _menuChangeVga;
        Gtk::MenuItem _menuAttachHdd;
        Gtk::MenuItem _menuAttachCd;
        Gtk::MenuItem _menuAttachNic;
        Gtk::MenuItem _menuCancel;
        RefPtr<VirtualMachine> _vm;
        Glib::ustring _name;
    };
}


#endif //!HNRT_VIRTUALMACHINEGENLVMENU_H
