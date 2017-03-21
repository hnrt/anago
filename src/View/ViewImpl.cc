// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "App/Constants.h"
#include "Base/StringBuffer.h"
#include "Controller/Controller.h"
#include "Logger/Trace.h"
#include "Model/ConnectSpec.h"
#include "Model/Model.h"
#include "XenServer/XenObject.h"
#include "ConnectDialog.h"
#include "PixStore.h"
#include "ViewImpl.h"


using namespace hnrt;


ViewImpl::ViewImpl()
    : _displayName("Anago")
{
    Trace trace("ViewImpl::ctor");
    Controller::instance().signalNotified(XenObject::CREATED).connect(sigc::mem_fun(*this, &ViewImpl::onObjectCreated));
}


ViewImpl::~ViewImpl()
{
    Trace trace("ViewImpl::dtor");
}


void ViewImpl::configure()
{
    Trace trace("ViewImpl::configure");
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
}


void ViewImpl::onObjectCreated(RefPtr<RefObj> object, int what)
{
    RefPtr<XenObject> xenObject = RefPtr<XenObject>::castStatic(object);
    if (_mainWindow.addObject(xenObject))
    {
        Controller::instance().signalNotified(object).connect(sigc::mem_fun(*this, &ViewImpl::onObjectUpdated));
    }
}


void ViewImpl::onObjectUpdated(RefPtr<RefObj> object, int what)
{
    RefPtr<XenObject> xenObject = RefPtr<XenObject>::castStatic(object);
    if (what == XenObject::DESTROYED)
    {
        _mainWindow.removeObject(xenObject);
    }
    else
    {
        _mainWindow.updateObject(xenObject, what);
    }
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


void ViewImpl::showBusyServers(const std::list<Glib::ustring>& names)
{
    StringBuffer message;
    message =
        names.size() > 1 ?
        gettext("The following servers are busy now.\n") :
        gettext("The following server is busy now.\n");
    for (std::list<Glib::ustring>::const_iterator iter = names.begin(); iter != names.end(); iter++)
    {
        message += "\n";
        message += (*iter).c_str();
    }
    showWarning(message.str());
}
