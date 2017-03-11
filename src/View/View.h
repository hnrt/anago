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

        virtual Gtk::Window& getWindow() = 0;
    };
}


#endif //!HNRT_VIEW_H
