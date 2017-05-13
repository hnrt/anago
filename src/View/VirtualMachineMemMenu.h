// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEMEMLVMENU_H
#define HNRT_VIRTUALMACHINEMEMLVMENU_H


#include "Base/RefPtr.h"
#include "NameValueMenu.h"


namespace hnrt
{
    class VirtualMachine;

    class VirtualMachineMemMenu
        : public NameValueMenu
    {
    public:

        VirtualMachineMemMenu(const RefPtr<VirtualMachine>&);
        virtual void popup(guint, guint32);
        virtual void popup(guint, guint32, const Glib::ustring&);

    protected:

        VirtualMachineMemMenu(const VirtualMachineMemMenu&);
        void operator =(const VirtualMachineMemMenu&);
        void onDeactivate();
        void onSelectionDone();
        void onChange();

        Gtk::MenuItem _menuChange;
        Gtk::MenuItem _menuCancel;
        RefPtr<VirtualMachine> _vm;
        Glib::ustring _name;
    };
}


#endif //!HNRT_VIRTUALMACHINEMEMLVMENU_H
