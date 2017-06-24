// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <stdexcept>
#include "App/Constants.h"
#include "Base/StringBuffer.h"
#include "Controller/SignalManager.h"
#include "Logger/Trace.h"
#include "Model/ConnectSpec.h"
#include "Model/Model.h"
#include "XenServer/CifsSpec.h"
#include "XenServer/HardDiskDriveSpec.h"
#include "XenServer/PerformanceMonitor.h"
#include "XenServer/Session.h"
#include "XenServer/VirtualMachine.h"
#include "AboutDialog.h"
#include "AddCifsDialog.h"
#include "AddVmDialog.h"
#include "AttachCdDialog.h"
#include "AttachHddDialog.h"
#include "AttachNicDialog.h"
#include "ChangeCdDialog.h"
#include "ConnectDialog.h"
#include "CopyVmDialog.h"
#include "CpuDialog.h"
#include "DeleteVmDialog.h"
#include "ExportVmDialog.h"
#include "HardDiskDriveSpecDialog.h"
#include "ImportVmDialog.h"
#include "MemoryDialog.h"
#include "NameDialog.h"
#include "PixStore.h"
#include "ResizeDialog.h"
#include "ShadowMemoryDialog.h"
#include "VgaDialog.h"
#include "ViewImpl.h"


using namespace hnrt;


ViewImpl::ViewImpl()
    : _displayName("Anago")
    , _mainWindow()
    , _statusWindow(_mainWindow)
{
    Trace trace(NULL, "ViewImpl::ctor");
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
    Trace trace(NULL, "ViewImpl::dtor");
}


void ViewImpl::load()
{
    Trace trace(NULL, "ViewImpl::load");
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
    Trace trace(NULL, "ViewImpl::save");
    Model::instance().setWidth(_mainWindow.getWidth());
    Model::instance().setHeight(_mainWindow.getHeight());
    Model::instance().setPane1Width(_mainWindow.getPane1Width());
}


void ViewImpl::clear()
{
    Trace trace(NULL, "ViewImpl::clear");
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


bool ViewImpl::askYesNo(const Glib::ustring& message)
{
    Gtk::MessageDialog dialog(_mainWindow, message, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
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


bool ViewImpl::getShadowMemorySettings(double& multiplier)
{
    ShadowMemoryDialog dialog(_mainWindow);
    dialog.setMultiplier(multiplier);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        multiplier = dialog.getMultiplier();
        return true;
    }
    else
    {
        return false;
    }

}


bool ViewImpl::getVgaSettings(bool& stdVga, int& ram)
{
    VgaDialog dialog(_mainWindow);
    dialog.setStdVga(stdVga);
    dialog.setVideoRam(ram > 0 ? ram : 8);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        stdVga = dialog.isStdVga();
        ram = dialog.getVideoRam();
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::selectCd(const VirtualMachine& vm, Glib::ustring& device, Glib::ustring& disc)
{
    ChangeCdDialog dialog(_mainWindow, vm);
    dialog.select(device);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        device = dialog.getDevice();
        disc = dialog.getImage();
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::getVirtualMachineSpec(const Session& session, VirtualMachineSpec& spec)
{
    AddVmDialog dialog(_mainWindow, session);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        dialog.getSpec(spec);
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::getVirtualMachineToCopy(const Session& session, Glib::ustring& label, Glib::ustring& sr)
{
    CopyVmDialog dialog(_mainWindow, session, label.c_str(), sr.c_str());
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        label = dialog.getName();
        if (dialog.isCopy())
        {
            sr = dialog.getSr();
        }
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::getDisksToDelete(const VirtualMachine& vm, std::list<Glib::ustring>& disks)
{
    DeleteVmDialog dialog(_mainWindow, vm);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        dialog.getDisks(disks);
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::getExportVmPath(Glib::ustring& path, bool& verify)
{
    ExportVmDialog dialog(_mainWindow);
    dialog.setPath(path);
    dialog.setVerify(verify);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_OK)
    {
        path = dialog.getPath();
        verify = dialog.getVerify();
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::getImportVmPath(Glib::ustring& path)
{
    ImportVmDialog dialog(_mainWindow);
    dialog.setPath(path);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_OK)
    {
        path = dialog.getPath();
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::getVerifyVmPath(Glib::ustring& path)
{
    ImportVmDialog dialog(_mainWindow);
    dialog.set_title(gettext("Virtual machine - Verify"));
    dialog.setPath(path);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_OK)
    {
        path = dialog.getPath();
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::getHddToAttach(const VirtualMachine& vm, Glib::ustring& userdevice, Glib::ustring& vdi)
{
    AttachHddDialog dialog(_mainWindow, vm);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        userdevice = dialog.getUserDevice();
        vdi = dialog.getVdi();
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::getCdToAttach(const VirtualMachine& vm, Glib::ustring& userdevice)
{
    AttachCdDialog dialog(_mainWindow, vm);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        userdevice = dialog.getUserdevice();
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::getHddToCreate(const Session& session, HardDiskDriveSpec& spec)
{
    HardDiskDriveSpecDialog dialog(_mainWindow, session, Glib::ustring(gettext("Add hard disk drive to SR")));
    dialog.setValue(spec);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        dialog.getValue(spec);
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::getSize(int64_t& size)
{
    ResizeDialog dialog(_mainWindow);
    dialog.setSize(size);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        size = dialog.getSize();
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::getNicToAttach(const VirtualMachine& vm, Glib::ustring& device, Glib::ustring& network)
{
    AttachNicDialog dialog(_mainWindow, vm);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        device = dialog.getDevice();
        network = dialog.getNetwork();
        return true;
    }
    else
    {
        return false;
    }
}


bool ViewImpl::getCifsToCreate(CifsSpec& spec)
{
    AddCifsDialog dialog(_mainWindow);
    dialog.setSpec(spec);
    int response = dialog.run();
    if (response == Gtk::RESPONSE_APPLY)
    {
        dialog.getSpec(spec);
        return true;
    }
    else
    {
        return false;
    }
}
