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
#include "Thread/ThreadManager.h"
#include "View/View.h"
#include "ControllerImpl.h"


using namespace hnrt;


void ControllerImpl::addCifs()
{
    TRACE("ControllerImpl::addCifs");
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
    TRACE("ControllerImpl::addCifsInBackground");
    Session& session = host->getSession();
    Session::Lock lock(session);
    XenObject::Busy busy1(session);
    XenObject::Busy busy2(*host);
    host->setDisplayStatus(gettext("Creating CIFS..."));
    XenRef<xen_sr, xen_sr_free_t> sr;
    if (!XenServer::addCifs(session, host->getHandle(), spec, &sr))
    {
        session.emit(XenObject::ERROR);
    }
}


void ControllerImpl::deleteCifs()
{
    TRACE("ControllerImpl::deleteCifs");
    std::list<RefPtr<StorageRepository> > list;
    if (!Model::instance().getSelected(list))
    {
        return;
    }
    std::list<RefPtr<StorageRepository> >::iterator next;
    for (std::list<RefPtr<StorageRepository> >::iterator iter = list.begin(); iter != list.end(); iter = next)
    {
        RefPtr<StorageRepository>& sr = *iter;
        if (sr->isBusy() || !sr->isCifs())
        {
            next = list.erase(iter);
        }
        else
        {
            next = ++iter;
        }
    }
    if (list.size() != 1)
    {
        return;
    }
    RefPtr<StorageRepository>& sr = list.front();
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
    TRACE("ControllerImpl::deleteCifsInBackground");
    TRACEPUT("Removing %s...", sr->getName().c_str());
    sr->remove();
}


void ControllerImpl::changeSrName()
{
    TRACE("ControllerImpl::changeSrName");
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
    TRACE("ControllerImpl::setDefaultSr");
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
    TRACE("ControllerImpl::addHdd");
    std::list<RefPtr<StorageRepository> > srList;
    if (Model::instance().getSelected(srList) == 1)
    {
        ControllerImpl::addHddTo(*srList.front());
    }
}


void ControllerImpl::addHddTo(StorageRepository& sr)
{
    TRACE("ControllerImpl::addHddTo");
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
    TRACE("ControllerImpl::addHddInBackground");
    Session& session = sr->getSession();
    XenObject::Busy busy(session);
    Session::Lock lock(session);
    XenRef<xen_vdi, xen_vdi_free_t> vdi;
    if (!XenServer::createVdi(session, spec, &vdi))
    {
        session.emit(XenObject::ERROR);
    }
}
