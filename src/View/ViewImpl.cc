// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "App/Constants.h"
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Model/ConnectSpec.h"
#include "Model/Model.h"
#include "ConnectDialog.h"
#include "PixStore.h"
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


bool ViewImpl::getConnectSpec(ConnectSpec& cs)
{
    bool edit = cs.hostname.bytes() ? true : false;
    const char* title = edit ? gettext("Edit host") : gettext("Add host");
    ConnectDialog dialog(_mainWindow, title, Gtk::Stock::OK);
    if (edit)
    {
        dialog.select(cs);
    }
    int response = dialog.run();
    if (response == Gtk::RESPONSE_OK)
    {
        cs = dialog.getConnectSpec();
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::confirmServerToRemove(const char* name)
{
    StringBuffer message;
    message.format(gettext("Do you really wish to remove the following host from the list?\n"));
    message += "\n";
    message += name;
    Gtk::MessageDialog dialog(_mainWindow, message.str(), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
    dialog.set_title(_displayName);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_YES)
    {
        return true;
    }
    else
    {
        return false;
    }
}
