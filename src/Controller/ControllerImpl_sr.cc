// Copyright (C) 2012-2017 Hideaki Narita


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
    //TODO: IMPLEMENT
}


void ControllerImpl::setDefaultSr()
{
    RefPtr<StorageRepository> sr = Model::instance().getSelectedSr();
    if (!sr || sr->isBusy() || sr->isTools() || sr->isCifs() || sr->getSubType() != StorageRepository::USR)
    {
        return;
    }
    Session& session = sr->getSession();
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
