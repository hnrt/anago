// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIEWIMPL_H
#define HNRT_VIEWIMPL_H


#include "MainWindow.h"
#include "View.h"


namespace hnrt
{
    class ViewImpl
        : public View
    {
    public:

        ViewImpl();
        ~ViewImpl();
        virtual const Glib::ustring& getDisplayName() { return _displayName; }
        virtual Gtk::Window& getWindow() { return _mainWindow; }
        virtual void showInfo(const Glib::ustring&);
        virtual void showWarning(const Glib::ustring&);
        virtual void showError(const Glib::ustring&);

    private:

        ViewImpl(const ViewImpl&);
        void operator =(const ViewImpl&);
        void showMessageDialog(const Glib::ustring&, Gtk::MessageType);

        Glib::ustring _displayName;
        MainWindow _mainWindow;
    };
}


#endif //!HNRT_VIEWIMPL_H
