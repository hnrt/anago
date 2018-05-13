// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_NAMEVALUEMENU_H
#define HNRT_NAMEVALUEMENU_H


#include <gtkmm.h>


namespace hnrt
{
    class NameValueMenu
        : public Gtk::Menu
    {
    public:

        NameValueMenu()
        {
        }

        virtual ~NameValueMenu()
        {
        }

        virtual void popup(guint, guint32, const Glib::ustring&) = 0;

    protected:

        NameValueMenu(const NameValueMenu&);
        void operator =(const NameValueMenu&);
    };
}


#endif //!HNRT_NAMEVALUEMENU_H
