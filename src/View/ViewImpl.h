// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIEWIMPL_H
#define HNRT_VIEWIMPL_H


#include "Logger/Logger.h"
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
        virtual Gtk::Window& getWindow() { return _mainWindow; }

    private:

        ViewImpl(const ViewImpl&);
        void operator =(const ViewImpl&);

        MainWindow _mainWindow;
        Logger& _log;
    };
}


#endif //!HNRT_VIEWIMPL_H
