// Copyright (C) 2012-2017 Hideaki Narita


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
    //TODO: IMPLEMENT
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
