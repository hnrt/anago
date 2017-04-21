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
#include "XenServer/VirtualBlockDevice.h"
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
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm || vm->isBusy())
    {
        return;
    }
    Glib::ustring device;
    Glib::ustring disc;
    if (!View::instance().selectCd(*vm, device, disc))
    {
        return;
    }
    Session& session = vm->getSession();
    Session::Lock lock(session);
    vm->changeCd((xen_vbd)device.c_str(), (xen_vdi)disc.c_str());
}


void ControllerImpl::changeCd2(const VirtualBlockDevice& vbd)
{
    RefPtr<VirtualMachine> vm = vbd.getVm();
    if (!vm || vm->isBusy())
    {
        return;
    }
    Glib::ustring device;
    Glib::ustring disc;
    if (!View::instance().selectCd(*vm, device, disc))
    {
        return;
    }
    Session& session = vm->getSession();
    Session::Lock lock(session);
    vm->changeCd((xen_vbd)device.c_str(), (xen_vdi)disc.c_str());
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
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm || vm->isBusy())
    {
        return;
    }
    XenPtr<xen_vm_record> record = vm->getRecord();
    Glib::ustring label(record->name_label);
    Glib::ustring description(record->name_description);
    if (!View::instance().getName(gettext("Change VM label/description"), label, description))
    {
        return;
    }
    Session& session = vm->getSession();
    Session::Lock lock(session);
    vm->setName(label.c_str(), description.c_str());
}


void ControllerImpl::changeCpu()
{
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm || vm->isBusy())
    {
        return;
    }
    XenPtr<xen_vm_record> record = vm->getRecord();
    int64_t vcpusMax = record->vcpus_max;
    int64_t vcpusAtStartup = record->vcpus_at_startup;
    int coresPerSocket = vm->getCoresPerSocket();
    while (View::instance().getCpuSettings(vcpusMax, vcpusAtStartup, coresPerSocket))
    {
        Session& session = vm->getSession();
        Session::Lock lock(session);
        if (vm->setVcpu(vcpusMax, vcpusAtStartup, coresPerSocket))
        {
            return;
        }
    }
}


void ControllerImpl::changeMemory()
{
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm || vm->isBusy())
    {
        return;
    }
    XenPtr<xen_vm_record> record = vm->getRecord();
    int64_t staticMin = record->memory_static_min;
    int64_t staticMax = record->memory_static_max;
    int64_t dynamicMin = record->memory_dynamic_min;
    int64_t dynamicMax = record->memory_dynamic_max;
    while (View::instance().getMemorySettings(staticMin, staticMax, dynamicMin, dynamicMax))
    {
        Session& session = vm->getSession();
        Session::Lock lock(session);
        if (vm->setMemory(staticMin, staticMax, dynamicMin, dynamicMax))
        {
            return;
        }
    }
}


void ControllerImpl::changeShadowMemory()
{
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm || vm->isBusy())
    {
        return;
    }
    XenPtr<xen_vm_record> record = vm->getRecord();
    double multiplier = record->hvm_shadow_multiplier;
    while (View::instance().getShadowMemorySettings(multiplier))
    {
        Session& session = vm->getSession();
        Session::Lock lock(session);
        if (vm->setShadowMemory(multiplier))
        {
            return;
        }
    }
}


void ControllerImpl::changeVga()
{
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm || vm->isBusy())
    {
        return;
    }
    XenPtr<xen_vm_record> record = vm->getRecord();
    bool stdVga = vm->isStdVga();
    int ram = vm->getVideoRam();
    if (ram <= 0)
    {
        ram = 8;
    }
    while (View::instance().getVgaSettings(stdVga, ram))
    {
        Session& session = vm->getSession();
        Session::Lock lock(session);
        if (vm->setStdVga(stdVga) && vm->setVideoRam(stdVga ? ram : 0))
        {
            return;
        }
    }
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
