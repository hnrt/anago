// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALDISKIMAGEMENU_H
#define HNRT_VIRTUALDISKIMAGEMENU_H


#include "NameValueMenu.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class VirtualDiskImage;

    class VirtualDiskImageMenu
        : public NameValueMenu
    {
    public:

        VirtualDiskImageMenu();
        VirtualDiskImageMenu(VirtualDiskImage&);
        void popup(guint, guint32, VirtualDiskImage&);
        virtual void popup(guint, guint32, const Glib::ustring&);

    protected:

        VirtualDiskImageMenu(const VirtualDiskImageMenu&);
        void operator =(const VirtualDiskImageMenu&);
        void init();
        void onDeactivate();
        void onSelectionDone();
        void onChange();
        void onChangeName();
        void onResize();
        void onRemove();

        Gtk::MenuItem _menuChange;
        Gtk::MenuItem _menuChangeName;
        Gtk::MenuItem _menuResize;
        Gtk::MenuItem _menuRemove;
        Gtk::MenuItem _menuCancel;
        RefPtr<VirtualDiskImage> _vdi;
        Glib::ustring _name;
    };
}


#endif //!HNRT_VIRTUALDISKIMAGEMENU_H
