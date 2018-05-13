// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_ABOUTDIALOG_H
#define HNRT_ABOUTDIALOG_H


#include <gtkmm.h>


namespace hnrt
{
    class AboutDialog
        : public Gtk::AboutDialog
    {
    public:

        AboutDialog();
        void move(Gtk::Window&);

    private:

        AboutDialog(const AboutDialog&);
        void operator =(const AboutDialog&);
    };
}


#endif //!HNRT_ABOUTDIALOG_H
