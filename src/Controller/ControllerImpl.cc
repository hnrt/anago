// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <stdexcept>
#include "Base/Atomic.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Model/ThreadManager.h"
#include "View/View.h"
#include "XenServer/Host.h"
#include "XenServer/PhysicalBlockDevice.h"
#include "XenServer/Session.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/VirtualBlockDevice.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObject.h"
#include "Background.h"
#include "ControllerImpl.h"


using namespace hnrt;


ControllerImpl::ControllerImpl()
    : _quitInProgress(false)
{
    Trace trace("ControllerImpl::ctor");
}


ControllerImpl::~ControllerImpl()
{
    Trace trace("ControllerImpl::dtor");
}


void ControllerImpl::clear()
{
    Trace trace("ControllerImpl::clear");
    Glib::RecMutex::Lock k(_mutex);
    _notified.clear();
    _notificationSignalMap.clear();
    _refObjSignalMap.clear();
}


void ControllerImpl::parseCommandLine(int argc, char *argv[])
{
    Trace trace("ControllerImpl::parseCommandLine");

    _dispatcher.connect(sigc::mem_fun(*this, &ControllerImpl::onNotify));

    for (int index = 1; index < argc; index++)
    {
        if (!strcmp(argv[index], "-log"))
        {
            if (++index == argc || argv[index][0] == '-')
            {
                static char msg[256];
                snprintf(msg, sizeof(msg), "Command line option -log operand was not specified.");
                throw std::runtime_error(msg);
            }
            Logger::instance().setLevel(LogLevel::parse(argv[index]));
        }
        else
        {
            static char msg[256];
            snprintf(msg, sizeof(msg), "Bad command line syntax: %s", argv[index]);
            throw std::runtime_error(msg);
        }
    }
}


void ControllerImpl::quit()
{
    Trace trace("ControllerImpl::quit");

    if (!_quitInProgress)
    {
        _quitInProgress = true;
        View::instance().getWindow().set_title(gettext("Quitting..."));
        if (quit2())
        {
            Glib::signal_timeout().connect(sigc::mem_fun(*this, &ControllerImpl::quit2), 100); // 100 milleseconds
        }
    }
}


bool ControllerImpl::quit2()
{
    Trace trace("ControllerImpl::quit2");

    int busyCount = 0;
    std::list<RefPtr<Host> > hosts;
    Model::instance().get(hosts);
    for (std::list<RefPtr<Host> >::iterator iter = hosts.begin(); iter != hosts.end(); iter++)
    {
        RefPtr<Host>& host = *iter;
        if (host->isBusy())
        {
            busyCount++;
        }
        else
        {
            Session& session = host->getSession();
            if (session.isConnected())
            {
                if (session.disconnect())
                {
                    host->onDisconnected();
                }
            }
        }
    }
    hosts.clear();
    int backgroundCount = ThreadManager::instance().count();
    if (busyCount || backgroundCount)
    {
        trace.put("busy=%d background=%d", busyCount, backgroundCount);
        return true; // to be invoked again
    }
    else
    {
        View::instance().getWindow().hide();
        return false; // done
    }
}


Controller::Signal ControllerImpl::signalNotified(int notification)
{
    Glib::RecMutex::Lock k(_mutex);
    NotificationSignalMap::iterator iter = _notificationSignalMap.find(notification);
    if (iter == _notificationSignalMap.end())
    {
        _notificationSignalMap.insert(NotificationSignalMapEntry(notification, Signal()));
        iter = _notificationSignalMap.find(notification);
    }
    return iter->second;
}


Controller::Signal ControllerImpl::signalNotified(const RefPtr<RefObj>& object)
{
    Glib::RecMutex::Lock k(_mutex);
    void* key = const_cast<RefObj*>(object.ptr());
    RefObjSignalMap::iterator iter = _refObjSignalMap.find(key);
    if (iter == _refObjSignalMap.end())
    {
        _refObjSignalMap.insert(RefObjSignalMapEntry(key, Signal()));
        iter = _refObjSignalMap.find(key);
    }
    return iter->second;
}


void ControllerImpl::notify(const RefPtr<RefObj>& object, int notification)
{
    Trace trace("ControllerImpl::notify", "object=%zx notification=%d", object.ptr(), notification);
    if (_quitInProgress)
    {
        trace.put("cancelled.");
        return;
    }
    Glib::RecMutex::Lock k(_mutex);
    _notified.push_back(RefPtrNotificationPair(object, notification));
    if (_notified.size() == 1)
    {
        _dispatcher();
    }
}


void ControllerImpl::onNotify()
{
    Trace trace("ControllerImpl::onNotify");

    for (;;)
    {
        RefPtrNotificationPair entry;
        {
            Glib::RecMutex::Lock k(_mutex);
            if (!_notified.size())
            {
                break;
            }
            entry = _notified.front();
            _notified.pop_front();
        }
        if (entry.second == XenObject::CREATED)
        {
            onObjectCreated(entry.first);
            continue;
        }
        {
            int key = entry.second;
            NotificationSignalMap::iterator iter = _notificationSignalMap.find(key);
            if (iter != _notificationSignalMap.end())
            {
                iter->second.emit(entry.first, entry.second);
            }
        }
        {
            void* key = entry.first.ptr();
            RefObjSignalMap::iterator iter = _refObjSignalMap.find(key);
            if (iter != _refObjSignalMap.end())
            {
                iter->second.emit(entry.first, entry.second);
                if (entry.second == XenObject::DESTROYED)
                {
                    _refObjSignalMap.erase(key);
                }
            }
        }
    }
}


void ControllerImpl::onObjectCreated(RefPtr<RefObj> object)
{
    RefPtr<XenObject> xenObject = RefPtr<XenObject>::castStatic(object);
    if (View::instance().addObject(xenObject))
    {
        signalNotified(object).connect(sigc::mem_fun(*this, &ControllerImpl::onObjectUpdated));
    }
}


void ControllerImpl::onObjectUpdated(RefPtr<RefObj> object, int what)
{
    RefPtr<XenObject> xenObject = RefPtr<XenObject>::castStatic(object);
    if (what == XenObject::DESTROYED)
    {
        View::instance().removeObject(xenObject);
    }
    else if (what == XenObject::CONNECT_FAILED)
    {
        Session& session = xenObject->getSession();
        StringBuffer message;
        message.format(gettext("Failed to connect to %s.\n"), session.getConnectSpec().hostname.c_str());
        XenServer::getError(session, message, "\n");
        View::instance().showWarning(message.str());
    }
    else
    {
        View::instance().updateObject(xenObject, what);
    }
}


void ControllerImpl::addHost()
{
    ConnectSpec cs;
    if (View::instance().getConnectSpec(cs))
    {
        Model::instance().add(cs);
    }
}


void ControllerImpl::editHost()
{
    RefPtr<Host> host = Model::instance().getSelectedHost();
    ConnectSpec cs = host->getSession().getConnectSpec();
    if (View::instance().getConnectSpec(cs))
    {
        Model::instance().add(cs);
    }
}


void ControllerImpl::removeHost()
{
    RefPtr<Host> host = Model::instance().getSelectedHost();
    Session& session = host->getSession();
    ConnectSpec cs = session.getConnectSpec();
    StringBuffer text;
    if (cs.displayname == cs.hostname)
    {
        text.format("%s", cs.displayname.c_str());
    }
    else
    {
        text.format("%s (%s)", cs.displayname.c_str(), cs.hostname.c_str());
    }
    if (View::instance().confirmServerToRemove(text))
    {
        Model::instance().remove(session);
    }
}


void ControllerImpl::connect()
{
    std::list<RefPtr<Host> > hosts;
    if (!Model::instance().getSelected(hosts))
    {
        return;
    }
    std::list<Glib::ustring> busyHosts;
    for (std::list<RefPtr<Host> >::iterator iter = hosts.begin(); iter != hosts.end(); iter++)
    {
        RefPtr<Host>& host = *iter;
        Session& session = host->getSession();
        if (session.isConnected())
        {
            continue;
        }
        else if (host->isBusy())
        {
            ConnectSpec& cs = session.getConnectSpec();
            busyHosts.push_back(cs.hostname);
            continue;
        }
        Glib::Thread::create(sigc::bind<RefPtr<Host> >(sigc::mem_fun(*this, &ControllerImpl::connectInBackground), host), false);
    }
    if (busyHosts.size())
    {
        View::instance().showBusyServers(busyHosts);
    }
}


void ControllerImpl::connectInBackground(RefPtr<Host> host)
{
    Background bg("Connect");
    Trace trace("ControllerImpl::connectInBackground", "host=%s", host->getSession().getConnectSpec().hostname.c_str());
    Session& session = host->getSession();
    {
        XenObject::Busy busy(host);
        host->onConnectPending();
        Session::Lock lock(session);
        if (session.connect())
        {
            trace.put("Connected successfully.");
            host->onConnected();
        }
        else
        {
            trace.put("Connect failed.");
            host->onConnectFailed();
            return;
        }
        //
        // enumerate all resources
        //
        XenPtr<xen_pbd_set> pbdSet;
        if (xen_pbd_get_all(session, pbdSet.address()))
        {
            for (size_t i = 0; i < pbdSet->size; i++)
            {
                XenPtr<xen_pbd_record> pbdRecord;
                if (xen_pbd_get_record(session, pbdRecord.address(), pbdSet->contents[i]))
                {
                    PhysicalBlockDevice::create(session, pbdSet->contents[i], pbdRecord);
                }
                else
                {
                    session.clearError();
                }
            }
        }
        else
        {
            session.clearError();
        }
        XenPtr<xen_vbd_set> vbdSet;
        if (xen_vbd_get_all(session, vbdSet.address()))
        {
            for (size_t i = 0; i < vbdSet->size; i++)
            {
                XenPtr<xen_vbd_record> vbdRecord;
                if (xen_vbd_get_record(session, vbdRecord.address(), vbdSet->contents[i]))
                {
                    VirtualBlockDevice::create(session, vbdSet->contents[i], vbdRecord);
                }
                else
                {
                    session.clearError();
                }
            }
        }
        else
        {
            session.clearError();
        }
        XenPtr<xen_vdi_set> vdiSet;
        if (xen_vdi_get_all(session, vdiSet.address()))
        {
            for (size_t i = 0; i < vdiSet->size; i++)
            {
                XenPtr<xen_vdi_record> vdiRecord;
                if (xen_vdi_get_record(session, vdiRecord.address(), vdiSet->contents[i]))
                {
                    VirtualDiskImage::create(session, vdiSet->contents[i], vdiRecord);
                }
                else
                {
                    session.clearError();
                }
            }
        }
        else
        {
            session.clearError();
        }
        XenPtr<xen_pif_set> pifSet;
        if (xen_pif_get_all(session, pifSet.address()))
        {
            for (size_t i = 0; i < pifSet->size; i++)
            {
                XenPtr<xen_pif_record> pifRecord;
                if (xen_pif_get_record(session, pifRecord.address(), pifSet->contents[i]))
                {
                    //PhysicalInterface::create(session, pifSet->contents[i], pifRecord);
                }
                else
                {
                    session.clearError();
                }
            }
        }
        else
        {
            session.clearError();
        }
        XenPtr<xen_network_set> nwSet;
        if (xen_network_get_all(session, nwSet.address()))
        {
            for (size_t i = 0; i < nwSet->size; i++)
            {
                XenPtr<xen_network_record> nwRecord;
                if (xen_network_get_record(session, nwRecord.address(), nwSet->contents[i]))
                {
                    //Network::create(session, nwSet->contents[i], nwRecord);
                }
                else
                {
                    session.clearError();
                }
            }
        }
        else
        {
            session.clearError();
        }
        XenPtr<xen_vif_set> vifSet;
        if (xen_vif_get_all(session, vifSet.address()))
        {
            for (size_t i = 0; i < vifSet->size; i++)
            {
                XenPtr<xen_vif_record> vifRecord;
                if (xen_vif_get_record(session, vifRecord.address(), vifSet->contents[i]))
                {
                    //VirtualInterface::create(session, vifSet->contents[i], vifRecord);
                }
                else
                {
                    session.clearError();
                }
            }
        }
        else
        {
            session.clearError();
        }
        XenPtr<xen_vm_set> vmSet;
        if (xen_vm_get_all(session, vmSet.address()))
        {
            for (size_t i = 0; i < vmSet->size; i++)
            {
                XenPtr<xen_vm_record> vmRecord;
                if (xen_vm_get_record(session, vmRecord.address(), vmSet->contents[i]))
                {
                    VirtualMachine::create(session, vmSet->contents[i], vmRecord);
                }
                else
                {
                    session.clearError();
                }
            }
        }
        else
        {
            session.clearError();
        }
        XenPtr<xen_sr_set> srSet;
        if (xen_sr_get_all(session, srSet.address()))
        {
            for (size_t i = 0; i < srSet->size; i++)
            {
                XenPtr<xen_sr_record> srRecord;
                if (xen_sr_get_record(session, srRecord.address(), srSet->contents[i]))
                {
                    StorageRepository::create(session, srSet->contents[i], srRecord);
                }
                else
                {
                    session.clearError();
                }
            }
        }
        else
        {
            session.clearError();
        }
    }
}


void ControllerImpl::disconnect()
{
    std::list<RefPtr<Host> > hosts;
    if (!Model::instance().getSelected(hosts))
    {
        return;
    }
    for (std::list<RefPtr<Host> >::iterator iter = hosts.begin(); iter != hosts.end(); iter++)
    {
        RefPtr<Host>& host = *iter;
        Session& session = host->getSession();
        if (!session.isConnected() || host->isBusy())
        {
            continue;
        }
        Glib::Thread::create(sigc::bind<RefPtr<Host> >(sigc::mem_fun(*this, &ControllerImpl::disconnectInBackground), host), false);
    }
}


void ControllerImpl::disconnectInBackground(RefPtr<Host> host)
{
    Background bg("Disconnect");
    Trace trace("ControllerImpl::disconnectInBackground", "host=%s", host->getSession().getConnectSpec().hostname.c_str());
    Session& session = host->getSession();
    XenObject::Busy busy(host);
    Session::Lock lock(session);
    if (session.disconnect())
    {
        trace.put("Disconnected successfully.");
        host->onDisconnected();
    }
    else
    {
        trace.put("Disconnect failed.");
    }
}


void ControllerImpl::showAbout()
{
    //TODO: IMPLEMENT
}
