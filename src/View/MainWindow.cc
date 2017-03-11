// Copyright (C) 2012-2017 Hideaki Narita


#include "Controller/Controller.h"
#include "MainWindow.h"


using namespace hnrt;


MainWindow::MainWindow()
{
    signal_delete_event().connect(sigc::mem_fun(*this, &MainWindow::onClose));
}


MainWindow::~MainWindow()
{
}


bool MainWindow::onClose(GdkEventAny* event)
{
    Controller::instance().quit();
    return true; // to stop other handlers from being invoked for this event
}
