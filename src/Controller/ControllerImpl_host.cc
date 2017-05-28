// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <stdexcept>
#include "Base/Atomic.h"
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Net/WakeOnLan.h"
#include "Thread/ThreadManager.h"
#include "View/View.h"
#include "XenServer/Host.h"
#include "XenServer/Network.h"
#include "XenServer/PerformanceMonitor.h"
#include "XenServer/PhysicalBlockDevice.h"
#include "XenServer/PhysicalInterface.h"
#include "XenServer/Session.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/VirtualBlockDevice.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/VirtualInterface.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenEventMonitor.h"
#include "XenServer/XenObject.h"
#include "XenServer/XenObjectStore.h"
#include "ControllerImpl.h"


using namespace hnrt;


void ControllerImpl::connectAtStartup()
{
    std::list<RefPtr<Host> > hosts;
    Model::instance().get(hosts);
    for (std::list<RefPtr<Host> >::iterator iter = hosts.begin(); iter != hosts.end(); iter++)
    {
        RefPtr<Host>& host = *iter;
        Session& session = host->getSession();
        ConnectSpec& cs = session.getConnectSpec();
        if (cs.autoConnect)
        {
            _tm.create(sigc::bind<RefPtr<Host> >(sigc::mem_fun(*this, &ControllerImpl::connectInBackground), host), false, "Connect");
        }
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
        _tm.create(sigc::bind<RefPtr<Host> >(sigc::mem_fun(*this, &ControllerImpl::connectInBackground), host), false, "Connect");
    }
    if (busyHosts.size())
    {
        View::instance().showBusyServers(busyHosts);
    }
}


void ControllerImpl::connectInBackground(RefPtr<Host> host)
{
    Trace trace("ControllerImpl::connectInBackground", "host=%s", host->getSession().getConnectSpec().hostname.c_str());
    Session& session = host->getSession();
    RefPtr<PerformanceMonitor> performanceMonitor = PerformanceMonitor::create(session);
    session.getStore().setPerformanceMonitor(performanceMonitor);
    {
        XenObject::Busy busy(*host);
        host->onConnectPending();
        Session::Lock lock(session);
        if (session.connect())
        {
            trace.put("Connected successfully.");
            ConnectSpec& cs = session.getConnectSpec();
            if (cs.mac.isNull())
            {
                cs.mac.getByName(cs.hostname.c_str());
            }
            if (!host->onConnected())
            {
                return;
            }
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
                    PhysicalInterface::create(session, pifSet->contents[i], pifRecord);
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
                    Network::create(session, nwSet->contents[i], nwRecord);
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
                    VirtualInterface::create(session, vifSet->contents[i], vifRecord);
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
        // To update snapshots...
        std::list<RefPtr<VirtualMachine> > vmList;
        if (session.getStore().getList(vmList))
        {
            for (std::list<RefPtr<VirtualMachine> >::iterator iter = vmList.begin(); iter != vmList.end(); iter++)
            {
                (*iter)->emit(XenObject::SNAPSHOT_UPDATED);
            }
        }
    }
    session.setMonitoring(true);
    Glib::Thread* pThead = _tm.create(sigc::bind<RefPtr<PerformanceMonitor> >(sigc::mem_fun(*this, &ControllerImpl::performanceMonitorInBackground), performanceMonitor), true, "PerformanceMonitor");
    XenEventMonitor eventMonitor;
    eventMonitor.run(session);
    performanceMonitor->terminate();
    pThead->join();
    session.getStore().removePerformanceMonitor();
    session.setMonitoring(false);
    if (session.isConnected())
    {
        disconnect(host);
    }
}


void ControllerImpl::performanceMonitorInBackground(RefPtr<PerformanceMonitor> performanceMonitor)
{
    Trace trace("ControllerImpl::performanceMonitorInBackground");
    performanceMonitor->run();
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
        if (session.isConnected() || !host->isBusy())
        {
            disconnect(host);
        }
    }
}


void ControllerImpl::disconnect(const RefPtr<Host>& host)
{
    schedule(sigc::bind<RefPtr<Host> >(sigc::mem_fun(*this, &ControllerImpl::disconnectInBackground), host));
}


void ControllerImpl::disconnectInBackground(RefPtr<Host> host)
{
    Trace trace("ControllerImpl::disconnectInBackground", "host=%s", host->getSession().getConnectSpec().hostname.c_str());
    XenObject::Busy busy(*host);
    host->onDisconnectPending();
    Session& session = host->getSession();
    Session::Lock lock(session);
    if (session.disconnect())
    {
        trace.put("Disconnected successfully.");
    }
    host->onDisconnected();
}


void ControllerImpl::changeHostName()
{
    RefPtr<Host> host = Model::instance().getSelectedHost();
    if (!host || host->isBusy() || !host->getSession() || !host->getRecord())
    {
        return;
    }
    XenPtr<xen_host_record> record = host->getRecord();
    Glib::ustring label(record->name_label);
    Glib::ustring description(record->name_description);
    if (!View::instance().getName(gettext("Change host label/description"), label, description))
    {
        return;
    }
    host->setName(label.c_str(), description.c_str());
}


void ControllerImpl::wakeHost()
{
    RefPtr<Host> host = Model::instance().getSelectedHost();
    if (!host)
    {
        return;
    }
    ConnectSpec& cs = host->getSession().getConnectSpec();
    if (cs.mac.isNull())
    {
        if (!cs.mac.getByName(cs.hostname.c_str()))
        {
            View::instance().showWarning(Glib::ustring::compose(gettext("MAC address is unavailable.\n\n%1"), cs.hostname));
            return;
        }
    }
    WakeOnLan wol(cs.mac);
    int rc = wol.send(cs.hostname.c_str());
    if (!rc)
    {
        View::instance().showInfo(
            Glib::ustring::compose(
                gettext("Sent a wake-on-LAN packet to %1.\n\nPlease wait a while for the host to start up."),
                cs.hostname));
    }
    else
    {
        View::instance().showWarning(
            Glib::ustring::compose(
                gettext("Unable to send a wake-on-LAN packet to %1.\n\n%2"),
                cs.hostname,
                strerror(rc)));
    }
}


void ControllerImpl::shutdownHosts()
{
    std::list<RefPtr<Host> > hosts;
    if (!Model::instance().getSelected(hosts))
    {
        return;
    }
    std::list<Glib::ustring> names;
    for (std::list<RefPtr<Host> >::iterator iter = hosts.begin(); iter != hosts.end(); iter++)
    {
        names.push_back((*iter)->getSession().getConnectSpec().hostname);
    }
    if (!View::instance().confirmServersToShutdown(names, false))
    {
        return;
    }
    for (std::list<RefPtr<Host> >::iterator iter = hosts.begin(); iter != hosts.end(); iter++)
    {
        schedule(sigc::bind<RefPtr<Host> >(sigc::mem_fun(*this, &ControllerImpl::shutdownHostInBackground), *iter));
    }
}


void ControllerImpl::shutdownHostInBackground(RefPtr<Host> host)
{
    Session& session = host->getSession();
    Session::Lock lock(session);
    host->shutdown();
}


void ControllerImpl::restartHosts()
{
    std::list<RefPtr<Host> > hosts;
    if (!Model::instance().getSelected(hosts))
    {
        return;
    }
    std::list<Glib::ustring> names;
    for (std::list<RefPtr<Host> >::iterator iter = hosts.begin(); iter != hosts.end(); iter++)
    {
        names.push_back((*iter)->getSession().getConnectSpec().hostname);
    }
    if (!View::instance().confirmServersToShutdown(names, true))
    {
        return;
    }
    for (std::list<RefPtr<Host> >::iterator iter = hosts.begin(); iter != hosts.end(); iter++)
    {
        schedule(sigc::bind<RefPtr<Host> >(sigc::mem_fun(*this, &ControllerImpl::restartHostInBackground), *iter));
    }
}


void ControllerImpl::restartHostInBackground(RefPtr<Host> host)
{
    Session& session = host->getSession();
    Session::Lock lock(session);
    host->reboot();
}
