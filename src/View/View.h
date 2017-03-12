// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIEW_H
#define HNRT_VIEW_H


#include <gtkmm.h>


namespace hnrt
{
    class View
    {
    public:

        static void init();
        static void fini();
        static View& instance();

        virtual const Glib::ustring& getDisplayName() = 0;
        virtual Gtk::Window& getWindow() = 0;
        virtual void showInfo(const Glib::ustring&) = 0;
        virtual void showWarning(const Glib::ustring&) = 0;
        virtual void showError(const Glib::ustring&) = 0;
    };
}


#endif //!HNRT_VIEW_H
