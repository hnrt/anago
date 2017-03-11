// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MAINWINDOW_H
#define HNRT_MAINWINDOW_H


#include <gtkmm.h>


namespace hnrt
{
    class MainWindow
        : public Gtk::Window
    {
    public:

        MainWindow();
        ~MainWindow();

    private:

        MainWindow(const MainWindow&);
        void operator =(const MainWindow&);
        bool onClose(GdkEventAny*);
    };
}


#endif //!HNRT_MAINWINDOW_H
