// Copyright (C) 2012-2017 Hideaki Narita


#include "App/Constants.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "ViewImpl.h"


using namespace hnrt;


ViewImpl::ViewImpl()
    : _displayName("Anago")
{
    Trace trace("ViewImpl::ctor");
}


ViewImpl::~ViewImpl()
{
    Trace trace("ViewImpl::dtor");
}


void ViewImpl::resize()
{
    Trace trace("ViewImpl::resize");
    int cx = Model::instance().getWidth();
    int cy = Model::instance().getHeight();
    if (cx > WIDTH_DEFAULT && cy > HEIGHT_DEFAULT)
    {
        trace.put("cx=%d cy=%d", cx, cy);
        _mainWindow.set_default_size(cx, cy);
    }
    cx = Model::instance().getPane1Width();
    if (cx > PANE1WIDTH_DEFAULT)
    {
        trace.put("pane1.cx=%d", cx);
        _mainWindow.setPane1Width(cx);
    }
}


void ViewImpl::clear()
{
    Trace trace("ViewImpl::clear");
    _mainWindow.clear();
    View::update();
}


void ViewImpl::showInfo(const Glib::ustring& message)
{
    showMessageDialog(message, Gtk::MESSAGE_INFO);
}


void ViewImpl::showWarning(const Glib::ustring& message)
{
    showMessageDialog(message, Gtk::MESSAGE_WARNING);
}


void ViewImpl::showError(const Glib::ustring& message)
{
    showMessageDialog(message, Gtk::MESSAGE_ERROR);
}


void ViewImpl::showMessageDialog(const Glib::ustring& message, Gtk::MessageType type)
{
    Gtk::MessageDialog dialog(_mainWindow, message, false, type);
    dialog.set_title(_displayName);
    dialog.run();
}
