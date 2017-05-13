// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Model/Model.h"
#include "XenServer/HardDiskDriveSpec.h"
#include "XenServer/Session.h"
#include "XenServer/StorageRepository.h"
#include "Thread/ThreadManager.h"
#include "View/View.h"
#include "ControllerImpl.h"


using namespace hnrt;


void ControllerImpl::addCifs()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::deleteCifs()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::changeSrName()
{
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
    std::list<RefPtr<StorageRepository> > srList;
    if (Model::instance().getSelected(srList) == 1)
    {
        ControllerImpl::addHddTo(*srList.front());
    }
}


void ControllerImpl::addHddTo(StorageRepository& sr)
{
    HardDiskDriveSpec spec;
    spec.srREFID = sr.getREFID();
    Session& session = sr.getSession();
    if (!View::instance().getHddToCreate(session, spec))
    {
        return;
    }
    _tm.create(sigc::bind<RefPtr<StorageRepository>, HardDiskDriveSpec>(sigc::mem_fun(*this, &ControllerImpl::addHddInBackground), RefPtr<StorageRepository>(&sr, 1), spec), false, "AddHdd");
}


void ControllerImpl::addHddInBackground(RefPtr<StorageRepository> sr, HardDiskDriveSpec spec)
{
    Session& session = sr->getSession();
    XenObject::Busy busy(session);
    Session::Lock lock(session);
    XenRef<xen_vdi, xen_vdi_free_t> vdi;
    if (!XenServer::createVdi(session, spec, &vdi))
    {
        session.emit(XenObject::ERROR);
    }
}
