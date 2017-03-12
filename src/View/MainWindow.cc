// Copyright (C) 2012-2017 Hideaki Narita


#include "Controller/Controller.h"
#include "Model/Model.h"
#include "MainWindow.h"


using namespace hnrt;


MainWindow::MainWindow()
    : _windowState((GdkWindowState)0)
{
    signal_delete_event().connect(sigc::mem_fun(*this, &MainWindow::onClose));
    signal_window_state_event().connect(sigc::mem_fun(*this, &MainWindow::onWindowStateChange));
    signal_size_allocate().connect(sigc::mem_fun(*this, &MainWindow::onResize));
}


MainWindow::~MainWindow()
{
}


bool MainWindow::onClose(GdkEventAny* event)
{
    Controller::instance().quit();
    return true; // to stop other handlers from being invoked for this event
}


bool MainWindow::onWindowStateChange(GdkEventWindowState* event)
{
    _windowState = event->new_window_state;
    return true;
}


void MainWindow::onResize(Gtk::Allocation& a)
{
    if (!(_windowState & (GDK_WINDOW_STATE_FULLSCREEN |
                          GDK_WINDOW_STATE_MAXIMIZED |
                          GDK_WINDOW_STATE_ICONIFIED)))
    {
        Model::instance().setWidth(a.get_width());
        Model::instance().setHeight(a.get_height());
    }
}
