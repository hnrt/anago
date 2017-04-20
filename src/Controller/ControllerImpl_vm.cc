// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <stdexcept>
#include "Base/Atomic.h"
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Thread/ThreadManager.h"
#include "View/View.h"
#include "XenServer/Session.h"
#include "XenServer/VirtualMachine.h"
#include "ControllerImpl.h"


using namespace hnrt;


void ControllerImpl::startVm()
{
    controlVm(&VirtualMachine::start);
}


void ControllerImpl::shutdownVm()
{
    controlVm(&VirtualMachine::shutdown, false);
}


void ControllerImpl::rebootVm()
{
    controlVm(&VirtualMachine::reboot, false);
}


void ControllerImpl::suspendVm()
{
    controlVm(&VirtualMachine::suspend);
}


void ControllerImpl::resumeVm()
{
    controlVm(&VirtualMachine::resume);
}


void ControllerImpl::controlVm(bool (VirtualMachine::*memfunc)())
{
    std::list<RefPtr<VirtualMachine> > vmList;
    if (!Model::instance().getSelected(vmList))
    {
        return;
    }
    for (std::list<RefPtr<VirtualMachine> >::iterator iter = vmList.begin(); iter != vmList.end(); iter++)
    {
        RefPtr<VirtualMachine>& vm = *iter;
        Session& session = vm->getSession();
        Session::Lock lock(session);
        (vm->*memfunc)();
    }
}


void ControllerImpl::controlVm(bool (VirtualMachine::*memfunc)(bool), bool arg1)
{
    std::list<RefPtr<VirtualMachine> > vmList;
    if (!Model::instance().getSelected(vmList))
    {
        return;
    }
    for (std::list<RefPtr<VirtualMachine> >::iterator iter = vmList.begin(); iter != vmList.end(); iter++)
    {
        RefPtr<VirtualMachine>& vm = *iter;
        Session& session = vm->getSession();
        Session::Lock lock(session);
        (vm->*memfunc)(arg1);
    }
}


void ControllerImpl::changeCd()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::sendCtrlAltDelete()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::addVm()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::copyVm()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::deleteVm()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::exportVm()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::importVm()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::verifyVm()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::hardShutdownVm()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::hardRebootVm()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::changeVmName()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::changeCpu()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::changeMemory()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::changeShadowMemory()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::changeVga()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::attachHdd()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::attachCd()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::attachNic()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::addHdd()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::changeSnapshotName()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::snapshotVm()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::revertVm()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::deleteSnapshot()
{
    //TODO: IMPLEMENT
}
