// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "XenServer/CifsSpec.h"
#include "XenServer/HardDiskDriveSpec.h"
#include "XenServer/Host.h"
#include "XenServer/Session.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/XenObjectStore.h"
#include "Thread/ThreadManager.h"
#include "View/View.h"
#include "ControllerImpl.h"


using namespace hnrt;


void ControllerImpl::addCifs()
{
    Trace trace(NULL, "ControllerImpl::addCifs");
    RefPtr<Host> host = Model::instance().getSelectedHost();
    if (!host || host->isBusy())
    {
        return;
    }
    CifsSpec spec;
    Model::instance().getCifsSpec(spec);
    bool retval = View::instance().getCifsToCreate(spec);
    Model::instance().setCifsSpec(spec);
    if (!retval)
    {
        return;
    }
    schedule(sigc::bind<RefPtr<Host>, CifsSpec>(sigc::mem_fun(*this, &ControllerImpl::addCifsInBackground), host, spec));
}


void ControllerImpl::addCifsInBackground(RefPtr<Host> host, CifsSpec spec)
{
    Trace trace(NULL, "ControllerImpl::addCifsInBackground(%s)", host->getSession().getConnectSpec().hostname.c_str());
    XenObject::Busy busy1(*host);
    Session& session = host->getSession();
    XenObject::Busy busy2(session);
    Session::Lock lock(session);
    host->setDisplayStatus(gettext("Creating CIFS..."));
    XenRef<xen_sr, xen_sr_free_t> sr;
    if (!XenServer::addCifs(session, host->getHandle(), spec, &sr))
    {
        session.emit(XenObject::ERROR);
    }
}


void ControllerImpl::deleteCifs()
{
    Trace trace(NULL, "ControllerImpl::deleteCifs");
    RefPtr<StorageRepository> sr = Model::instance().getSelectedSr();
    if (sr->isBusy() || !sr->isCifs())
    {
        return;
    }
    StringBuffer message;
    message.format(gettext("Do you wish to delete the following CIFS repository?\n\n%s (%s)"),
                   sr->getName().c_str(),
                   sr->getUUID().c_str());
    if (!View::instance().askYesNo(Glib::ustring(message)))
    {
        return;
    }
    schedule(sigc::bind<RefPtr<StorageRepository> >(sigc::mem_fun(*this, &ControllerImpl::deleteCifsInBackground), sr));
}


void ControllerImpl::deleteCifsInBackground(RefPtr<StorageRepository> sr)
{
    Trace trace(NULL, "ControllerImpl::deleteCifsInBackground");
    trace.put("Removing %s...", sr->getName().c_str());
    Session& session = sr->getSession();
    RefPtr<Host> host = session.getStore().getHost();
    XenObject::Busy busy1(*host);
    XenObject::Busy busy2(session);
    Session::Lock lock(session);
    host->setDisplayStatus(gettext("Deleting CIFS..."));
    sr->remove();
}


void ControllerImpl::changeSrName()
{
    Trace trace(NULL, "ControllerImpl::changeSrName");
    RefPtr<StorageRepository> sr = Model::instance().getSelectedSr();
    if (!sr || sr->isBusy())
    {
        return;
    }
    XenPtr<xen_sr_record> record = sr->getRecord();
    Glib::ustring label(record->name_label);
    Glib::ustring description(record->name_description);
    if (!View::instance().getName(gettext("Change SR label/description"), label, description))
    {
        return;
    }
    Session& session = sr->getSession();
    Session::Lock lock(session);
    sr->setName(label.c_str(), description.c_str());
}


void ControllerImpl::setDefaultSr()
{
    Trace trace(NULL, "ControllerImpl::setDefaultSr");
    RefPtr<StorageRepository> sr = Model::instance().getSelectedSr();
    if (!sr || sr->isBusy() || sr->isTools() || sr->isCifs() || sr->getSubType() != StorageRepository::USR)
    {
        return;
    }
    Session& session = sr->getSession();
    Session::Lock lock(session);
    if (!XenServer::setDefaultSr(session, sr->getHandle()))
    {
        session.emit(XenObject::ERROR);
    }
}


void ControllerImpl::addHdd()
{
    Trace trace(NULL, "ControllerImpl::addHdd");
    RefPtr<StorageRepository> sr;
    std::list<RefPtr<StorageRepository> > srList;
    if (Model::instance().getSelected(srList) == 1)
    {
        sr = srList.front();
    }
    else
    {
        RefPtr<Host> host = Model::instance().getSelectedHost();
        if (!host || host->isBusy())
        {
            return;
        }
        Session& session = host->getSession();
        Glib::ustring refid = XenServer::getDefaultSr(session);
        if (refid.empty())
        {
            return;
        }
        sr = session.getStore().getSr(refid);
    }
    if (!sr)
    {
        return;
    }
    ControllerImpl::addHddTo(*sr);
}


void ControllerImpl::addHddTo(StorageRepository& sr)
{
    Trace trace(NULL, "ControllerImpl::addHddTo");
    HardDiskDriveSpec spec;
    spec.srREFID = sr.getREFID();
    Session& session = sr.getSession();
    if (!View::instance().getHddToCreate(session, spec))
    {
        return;
    }
    RefPtr<StorageRepository> srPtr(&sr, 1);
    schedule(sigc::bind<RefPtr<StorageRepository>, HardDiskDriveSpec>(sigc::mem_fun(*this, &ControllerImpl::addHddInBackground), srPtr, spec));
}


void ControllerImpl::addHddInBackground(RefPtr<StorageRepository> sr, HardDiskDriveSpec spec)
{
    Trace trace(NULL, "ControllerImpl::addHddInBackground");
    Session& session = sr->getSession();
    XenObject::Busy busy(session);
    Session::Lock lock(session);
    XenRef<xen_vdi, xen_vdi_free_t> vdi;
    if (!XenServer::createVdi(session, spec, &vdi))
    {
        session.emit(XenObject::ERROR);
    }
}
