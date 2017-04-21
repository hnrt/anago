// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <stdexcept>
#include "App/Constants.h"
#include "Base/StringBuffer.h"
#include "Controller/SignalManager.h"
#include "Logger/Trace.h"
#include "Model/ConnectSpec.h"
#include "Model/Model.h"
#include "XenServer/PerformanceMonitor.h"
#include "XenServer/XenObject.h"
#include "AboutDialog.h"
#include "ConnectDialog.h"
#include "CpuDialog.h"
#include "MemoryDialog.h"
#include "NameDialog.h"
#include "PixStore.h"
#include "ViewImpl.h"


using namespace hnrt;


ViewImpl::ViewImpl()
    : _displayName("Anago")
{
    Trace trace("ViewImpl::ctor");
    Gdk::VisualType vt = Gdk::Visual::get_best_type();
    int depth = Gdk::Visual::get_best_depth();
    trace.put("visual=%s depth=%d",
              vt == Gdk::VISUAL_STATIC_GRAY ? "STATIC_GRAY" :
              vt == Gdk::VISUAL_GRAYSCALE ? "GRAYSCALE" :
              vt == Gdk::VISUAL_STATIC_COLOR ? "STATIC_COLOR" :
              vt == Gdk::VISUAL_PSEUDO_COLOR ? "PSEUDO_COLOR" :
              vt == Gdk::VISUAL_TRUE_COLOR ? "TRUE_COLOR" :
              vt == Gdk::VISUAL_DIRECT_COLOR ? "DIRECT_COLOR" :
              "?",
              depth);
    if (vt == Gdk::VISUAL_DIRECT_COLOR
        || vt == Gdk::VISUAL_TRUE_COLOR)
    {
        // OK
    }
    else
    {
        throw std::runtime_error("Current visual type not supported.");
    }
    if (depth == 32
        || depth == 24)
    {
        // OK
    }
    else
    {
        throw std::runtime_error("Current color depth not supported.");
    }
}


ViewImpl::~ViewImpl()
{
    Trace trace("ViewImpl::dtor");
}


void ViewImpl::load()
{
    Trace trace("ViewImpl::load");
    int cx = Model::instance().getWidth();
    int cy = Model::instance().getHeight();
    if (cx > WIDTH_DEFAULT && cy > HEIGHT_DEFAULT)
    {
        trace.put("cx=%d cy=%d", cx, cy);
        _mainWindow.setSize(cx, cy);
    }
    cx = Model::instance().getPane1Width();
    if (cx > PANE1WIDTH_DEFAULT)
    {
        trace.put("pane1.cx=%d", cx);
        _mainWindow.setPane1Width(cx);
    }
}


void ViewImpl::save()
{
    Trace trace("ViewImpl::save");
    Model::instance().setWidth(_mainWindow.getWidth());
    Model::instance().setHeight(_mainWindow.getHeight());
    Model::instance().setPane1Width(_mainWindow.getPane1Width());
}


void ViewImpl::clear()
{
    Trace trace("ViewImpl::clear");
    SignalManager::instance().clear();
    _mainWindow.clear();
}


void ViewImpl::about()
{
    AboutDialog dialog;
    dialog.move(_mainWindow);
    dialog.run();
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
        message += iter->c_str();
    }
    showWarning(message.str());
}


bool ViewImpl::confirmServersToShutdown(const std::list<Glib::ustring>& names, bool reboot)
{
    if (names.size() == 0)
    {
        return false;
    }
    StringBuffer message;
    if (reboot)
    {
        message =
            names.size() > 1 ?
            gettext("Do you wish to shut down and to restart the following hosts?\n") :
            gettext("Do you wish to shut down and to restart the following host?\n");
    }
    else
    {
        message =
            names.size() > 1 ?
            gettext("Do you wish to shut down the following hosts?\n") :
            gettext("Do you wish to shut down the following host?\n");
    }
    for (std::list<Glib::ustring>::const_iterator iter = names.begin(); iter != names.end(); iter++)
    {
        message += "\n";
        message += iter->c_str();
    }
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


bool ViewImpl::getName(const char* title, Glib::ustring& label, Glib::ustring& description)
{
    NameDialog dialog(_mainWindow, title);
    dialog.setLabel(label.c_str());
    dialog.setDescription(description.c_str());
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        label = dialog.getLabel();
        description = dialog.getDescription();
        return true;
    }
    else
    {
        return false;
    }

}


bool ViewImpl::getCpuSettings(int64_t& vcpusMax, int64_t& vcpusAtStartup, int& coresPerSocket)
{
    CpuDialog dialog(_mainWindow);
    dialog.setVcpusMax(vcpusMax);
    dialog.setVcpusAtStartup(vcpusAtStartup);
    dialog.setCoresPerSocket(coresPerSocket);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        vcpusMax = dialog.getVcpusMax();
        vcpusAtStartup = dialog.getVcpusAtStartup();
        coresPerSocket = dialog.getCoresPerSocket();
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::getMemorySettings(int64_t& staticMin, int64_t& staticMax, int64_t& dynamicMin, int64_t& dynamicMax)
{
    MemoryDialog dialog(_mainWindow);
    dialog.setStaticMin(staticMin);
    dialog.setStaticMax(staticMax);
    dialog.setDynamicMin(dynamicMin);
    dialog.setDynamicMax(dynamicMax);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        staticMin = dialog.getStaticMin();
        staticMax = dialog.getStaticMax();
        dynamicMin = dialog.getDynamicMin();
        dynamicMax = dialog.getDynamicMax();
        return true;
    }
    else
    {
        return false;
    }
}
