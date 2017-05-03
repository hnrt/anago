// Copyright (C) 2012-2017 Hideaki Narita


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
    //TODO: IMPLEMENT
#if 0
    HardDiskDriveSpec spec;
    spec.srREFID = sr.getREFID();
    Session& session = sr.getSession();
    if (!View::instance().getHddToCreate(session, spec))
    {
        return;
    }
    _tm.create(sigc::bind<RefPtr<StorageRepository>, HardDiskDriveSpec>(sigc::mem_fun(*this, &ControllerImpl::addHddInBackground), RefPtr<StrorageRepository>(&sr, 1), spec), false, "AddHdd");
#endif
}


#if 0
void ControllerImpl::addHddInBackground(RefPtr<StorageRepository> sr, HardDiskDriveSpec spec)
{
    Session& session = sr->getSession();
    Session::Lock lock(session);
    XenServer::createHdd(session, spec);
}
#endif
