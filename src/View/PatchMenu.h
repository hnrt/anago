// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_PATCHMENU_H
#define HNRT_PATCHMENU_H


#include <gtkmm.h>
#include "Model/PatchState.h"


namespace hnrt
{
    class PatchMenu
        : public Gtk::Menu
    {
    public:

        PatchMenu();
        virtual ~PatchMenu();
        virtual void popup(guint button, guint32 activate_time, const char* uuid, PatchState state);

    protected:

        PatchMenu(const PatchMenu&);
        void operator =(const PatchMenu&);
        void onDeactivate();
        void onSelectionDone();
        void onBrowse();
        void onDownload();
        void onUpload();
        void onApply();
        void onClean();

        Gtk::MenuItem _menuBrowse;
        Gtk::MenuItem _menuDownload;
        Gtk::MenuItem _menuUpload;
        Gtk::MenuItem _menuApply;
        Gtk::MenuItem _menuClean;
        Gtk::MenuItem _menuCancel;
        Glib::ustring _uuid;
    };
}


#endif //!HNRT_PATCHMENU_H
