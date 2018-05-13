// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEMENU_H
#define HNRT_VIRTUALMACHINEMENU_H


#include <gtkmm.h>


namespace hnrt
{
    class VirtualMachine;

    class VirtualMachineMenu
        : public Gtk::Menu
    {
    public:

        VirtualMachineMenu();
        void popup(guint, guint32, VirtualMachine& vm);

    protected:

        VirtualMachineMenu(const VirtualMachineMenu&);
        void operator =(const VirtualMachineMenu&);
        void onDeactivate();
        void onSelectionDone();

        Gtk::MenuItem _menuStart;
        Gtk::MenuItem _menuShutdown;
        Gtk::MenuItem _menuReboot;
        Gtk::MenuItem _menuSuspend;
        Gtk::MenuItem _menuResume;
        Gtk::MenuItem _menuChangeCd;
        Gtk::MenuItem _menuSendSas; // Secure Attention Sequence
        Gtk::MenuItem _menuChange;
        Gtk::MenuItem _menuCopy;
        Gtk::MenuItem _menuDelete;
        Gtk::MenuItem _menuExport;
        Gtk::MenuItem _menuCancel;
        Gtk::Menu _submenuChange;
        Gtk::MenuItem _menuChangeName;
        Gtk::MenuItem _menuChangeCpu;
        Gtk::MenuItem _menuChangeMemory;
        Gtk::MenuItem _menuChangeShadowMemory;
        Gtk::MenuItem _menuChangeVga;
        Gtk::MenuItem _menuAttachHdd;
        Gtk::MenuItem _menuAttachCd;
        Gtk::MenuItem _menuAttachNic;
    };
}


#endif //!HNRT_VIRTUALMACHINEMENU_H
