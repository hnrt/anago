// Copyright (C) 2012-2018 Hideaki Narita


#include <errno.h>
#include <libintl.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Base/Atomic.h"
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Net/Console.h"
#include "Thread/ThreadManager.h"
#include "View/View.h"
#include "XenServer/Host.h"
#include "XenServer/Session.h"
#include "XenServer/VirtualBlockDevice.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/VirtualInterface.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/VirtualMachineExporter.h"
#include "XenServer/VirtualMachineImporter.h"
#include "XenServer/VirtualMachineVerifier.h"
#include "XenServer/XenObjectStore.h"
#include "XenServer/XenServer.h"
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
    Glib::ustring device = vbd.getREFID();
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
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm)
    {
        return;
    }
    RefPtr<Console> console = Model::instance().getConsole(vm->getUUID());
    if (console)
    {
        console->sendCtrlAltDelete();
    }
}


void ControllerImpl::addVm()
{
    RefPtr<Host> host = Model::instance().getSelectedHost();
    if (!host || host->isBusy() || !host->getSession().isConnected())
    {
        return;
    }
    Session& session = host->getSession();
    VirtualMachineSpec spec;
    if (!View::instance().getVirtualMachineSpec(session, spec))
    {
        return;
    }
    schedule(sigc::bind<RefPtr<Host>, VirtualMachineSpec>(sigc::mem_fun(*this, &ControllerImpl::addVmInBackground), host, spec));
}


void ControllerImpl::addVmInBackground(RefPtr<Host> host, VirtualMachineSpec spec)
{
    Trace trace(NULL, "ControllerImpl::addVmInBackground");
    XenObject::Busy busy(*host);
    try
    {
        Session& session = host->getSession();
        Session::Lock lock(session);
        host->setDisplayStatus(gettext("Creating VM..."));
        XenRef<xen_vm, xen_vm_free_t> vm;
        if (XenServer::createVirtualMachine(session, spec, &vm))
        {
            XenPtr<xen_vm_record> vmRecord;
            if (xen_vm_get_record(session, vmRecord.address(), vm))
            {
                VirtualMachine::create(session, vm, vmRecord);
            }
            else
            {
                Logger::instance().error("%s: Getting VM record failed.", trace.name().data());
            }
        }
    }
    catch (...)
    {
        Logger::instance().error("%s: Unhandled exception caught.", trace.name().data());
    }
}


void ControllerImpl::copyVm()
{
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm || vm->isBusy())
    {
        return;
    }
    Session& session = vm->getSession();
    Glib::ustring label = vm->getRecord()->name_label;
    Glib::ustring srREFID = vm->getPrimarySr();
    if (!View::instance().getVirtualMachineToCopy(session, label, srREFID))
    {
        return;
    }
    if (srREFID.empty())
    {
        schedule(sigc::bind<RefPtr<VirtualMachine>, Glib::ustring>(sigc::mem_fun(*this, &ControllerImpl::cloneVmInBackground), vm, label));
    }
    else
    {
        schedule(sigc::bind<RefPtr<VirtualMachine>, Glib::ustring, Glib::ustring>(sigc::mem_fun(*this, &ControllerImpl::copyVmInBackground), vm, label, srREFID));
    }
}


void ControllerImpl::cloneVmInBackground(RefPtr<VirtualMachine> vm, Glib::ustring label)
{
    Trace trace(NULL, "ControllerImpl::cloneVmInBackground");
    XenObject::Busy busy(*vm);
    try
    {
        Session& session = vm->getSession();
        Session::Lock lock(session);
        // display status will be updated by task object
        vm->clone(label.c_str());
    }
    catch (...)
    {
        Logger::instance().error("%s: Unhandled exception caught.", trace.name().data());
    }
}


void ControllerImpl::copyVmInBackground(RefPtr<VirtualMachine> vm, Glib::ustring label, Glib::ustring srREFID)
{
    Trace trace(NULL, "ControllerImpl::copyVmInBackground");
    XenObject::Busy busy(*vm);
    try
    {
        Session& session = vm->getSession();
        Session::Lock lock(session);
        // display status will be updated by task object
        vm->copy(label.c_str(), (xen_sr)(char*)srREFID.c_str());
    }
    catch (...)
    {
        Logger::instance().error("%s: Unhandled exception caught.", trace.name().data());
    }
}


void ControllerImpl::deleteVm()
{
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm || vm->isBusy())
    {
        return;
    }
    std::list<Glib::ustring> disks;
    if (!View::instance().getDisksToDelete(*vm, disks))
    {
        return;
    }
    schedule(sigc::bind<RefPtr<VirtualMachine>, std::list<Glib::ustring> >(sigc::mem_fun(*this, &ControllerImpl::deleteVmInBackground), vm, disks));
}


void ControllerImpl::deleteVmInBackground(RefPtr<VirtualMachine> vm, std::list<Glib::ustring> disks)
{
    Session& session = vm->getSession();
    XenObjectStore& store = session.getStore();
    RefPtr<Host> host = store.getHost();
    XenObject::Busy busy(*host);
    Session::Lock lock(session);
    host->setDisplayStatus(gettext("Deleting VM..."));
    if (!vm->destroy())
    {
        return;
    }
    for (std::list<Glib::ustring>::const_iterator iter = disks.begin(); iter != disks.end(); iter++)
    {
        RefPtr<VirtualDiskImage> vdi = store.getVdi(*iter);
        if (!vdi->destroy())
        {
            return;
        }
    }
}


void ControllerImpl::exportVm()
{
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm || vm->isBusy())
    {
        return;
    }
    Glib::ustring path = Model::instance().getExportVmPath(*vm);
    bool verify = Model::instance().getExportVmVerify();
    while (View::instance().getExportVmPath(path, verify))
    {
        struct stat statinfo = { 0 };
        if (stat(path.c_str(), &statinfo))
        {
        doIt:
            schedule(sigc::bind<RefPtr<VirtualMachine>, Glib::ustring, bool>(sigc::mem_fun(*this, &ControllerImpl::exportVmInBackground), vm, path, verify));
            return;
        }
        else if (S_ISREG(statinfo.st_mode))
        {
            StringBuffer message;
            message.format(gettext("The file you just chose already exists.\n\n%s\n\nDo you really wish to overwrite?"), path.c_str());
            if (View::instance().askYesNo(Glib::ustring(message)))
            {
                goto doIt;
            }
        }
        else
        {
            StringBuffer message;
            message.format(gettext("The file you just chose isn't a regular file.\n\n%s"), path.c_str());
            View::instance().showWarning(Glib::ustring(message));
        }
    }
}


void ControllerImpl::exportVmInBackground(RefPtr<VirtualMachine> vm, Glib::ustring path, bool verify)
{
    Model::instance().setExportVmPath(path);
    Model::instance().setExportVmVerify(verify);
    RefPtr<VirtualMachineExporter> exporter = VirtualMachineExporter::create(vm);
    exporter->emit(XenObject::CREATED);
    exporter->run(path.c_str(), verify);
    exporter->emit(XenObject::DESTROYED);
}


void ControllerImpl::importVm()
{
    RefPtr<Host> host = Model::instance().getSelectedHost();
    if (!host || host->isBusy() || !host->getSession().isConnected())
    {
        return;
    }
    Glib::ustring path = Model::instance().getImportVmPath();
    while (View::instance().getImportVmPath(path))
    {
        struct stat statinfo = { 0 };
        if (stat(path.c_str(), &statinfo))
        {
            StringBuffer message;
            message.format(errno == ENOENT ? gettext("The file you just chose is not found.\n\n%s") :
                           gettext("The file you just chose cannot be read.\n\n%s"),
                           path.c_str());
            View::instance().showWarning(Glib::ustring(message));
        }
        else if (S_ISREG(statinfo.st_mode))
        {
            schedule(sigc::bind<RefPtr<Host>, Glib::ustring>(sigc::mem_fun(*this, &ControllerImpl::importVmInBackground), host, path));
            return;
        }
        else
        {
            StringBuffer message;
            message.format(gettext("The file you just chose isn't a regular file.\n\n%s"), path.c_str());
            View::instance().showWarning(Glib::ustring(message));
        }
    }
}


void ControllerImpl::importVmInBackground(RefPtr<Host> host, Glib::ustring path)
{
    Model::instance().setImportVmPath(path);
    RefPtr<VirtualMachineImporter> importer = VirtualMachineImporter::create(host->getSession());
    importer->emit(XenObject::CREATED);
    importer->run(path.c_str());
    importer->emit(XenObject::DESTROYED);
}


void ControllerImpl::verifyVm()
{
    Glib::ustring path = Model::instance().getVerifyVmPath();
    while (View::instance().getVerifyVmPath(path))
    {
        struct stat statinfo = { 0 };
        if (stat(path.c_str(), &statinfo))
        {
            StringBuffer message;
            message.format(errno == ENOENT ? gettext("The file you just chose is not found.\n\n%s") :
                           gettext("The file you just chose cannot be read.\n\n%s"),
                           path.c_str());
            View::instance().showWarning(Glib::ustring(message));
        }
        else if (S_ISREG(statinfo.st_mode))
        {
            schedule(sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &ControllerImpl::verifyVmInBackground), path));
            return;
        }
        else
        {
            StringBuffer message;
            message.format(gettext("The file you just chose isn't a regular file.\n\n%s"), path.c_str());
            View::instance().showWarning(Glib::ustring(message));
        }
    }
}


void ControllerImpl::verifyVmInBackground(Glib::ustring path)
{
    Model::instance().setVerifyVmPath(path);
    RefPtr<VirtualMachineVerifier> verifier = VirtualMachineVerifier::create();
    verifier->emit(XenObject::CREATED);
    verifier->run(path.c_str());
    verifier->emit(XenObject::DESTROYED);
}


void ControllerImpl::hardShutdownVm()
{
    controlVm(&VirtualMachine::shutdown, true);
}


void ControllerImpl::hardRebootVm()
{
    controlVm(&VirtualMachine::reboot, true);
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
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm || vm->isBusy())
    {
        return;
    }
    Glib::ustring userdevice;
    Glib::ustring vdi;
    if (!View::instance().getHddToAttach(*vm, userdevice, vdi))
    {
        return;
    }
    schedule(sigc::bind<RefPtr<VirtualMachine>, Glib::ustring, Glib::ustring>(sigc::mem_fun(*this, &ControllerImpl::attachHddInBackground), vm, userdevice, vdi));
}


void ControllerImpl::attachHddInBackground(RefPtr<VirtualMachine> vm, Glib::ustring userdevice, Glib::ustring vdi)
{
    XenObject::Busy busy(*vm);
    Session& session = vm->getSession();
    Session::Lock lock(session);
    vm->setDisplayStatus(gettext("Attaching HDD..."));
    if (!XenServer::attachHdd(session, vm->getHandle(), userdevice.c_str(), (xen_vdi)vdi.c_str()))
    {
        session.emit(XenObject::ERROR);
    }
}


void ControllerImpl::detachHdd(VirtualBlockDevice& vbd)
{
    StringBuffer message;
    message.format(gettext("Do you wish to detach %s?"), vbd.getDeviceName().c_str());
    if (!View::instance().askYesNo(Glib::ustring(message)))
    {
        return;
    }
    RefPtr<VirtualBlockDevice> vbdPtr(&vbd, 1);
    schedule(sigc::bind<RefPtr<VirtualBlockDevice> >(sigc::mem_fun(*this, &ControllerImpl::detachHddInBackground), vbdPtr));
}


void ControllerImpl::detachHddInBackground(RefPtr<VirtualBlockDevice> vbd)
{
    RefPtr<VirtualMachine> vm = vbd->getVm();
    if (!vm)
    {
        return;
    }
    XenObject::Busy busy(*vm);
    Session& session = vm->getSession();
    Session::Lock lock(session);
    vm->setDisplayStatus(gettext("Detaching HDD..."));
    if (!xen_vbd_destroy(session, vbd->getHandle()))
    {
        session.emit(XenObject::ERROR);
    }
}


void ControllerImpl::attachCd()
{
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm || vm->isBusy())
    {
        return;
    }
    Glib::ustring userdevice;
    if (!View::instance().getCdToAttach(*vm, userdevice))
    {
        return;
    }
    schedule(sigc::bind<RefPtr<VirtualMachine>, Glib::ustring>(sigc::mem_fun(*this, &ControllerImpl::attachCdInBackground), vm, userdevice));
}


void ControllerImpl::attachCdInBackground(RefPtr<VirtualMachine> vm, Glib::ustring userdevice)
{
    XenObject::Busy busy(*vm);
    Session& session = vm->getSession();
    Session::Lock lock(session);
    vm->setDisplayStatus(gettext("Attaching CD..."));
    if (!XenServer::attachCd(session, vm->getHandle(), userdevice.c_str()))
    {
        session.emit(XenObject::ERROR);
    }
}


void ControllerImpl::attachNic()
{
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm || vm->isBusy())
    {
        return;
    }
    Glib::ustring device;
    Glib::ustring network;
    if (!View::instance().getNicToAttach(*vm, device, network))
    {
        return;
    }
    schedule(sigc::bind<RefPtr<VirtualMachine>, Glib::ustring, Glib::ustring>(sigc::mem_fun(*this, &ControllerImpl::attachNicInBackground), vm, device, network));
}


void ControllerImpl::attachNicInBackground(RefPtr<VirtualMachine> vm, Glib::ustring device, Glib::ustring network)
{
    XenObject::Busy busy(*vm);
    Session& session = vm->getSession();
    Session::Lock lock(session);
    vm->setDisplayStatus(gettext("Attaching NIC..."));
    if (!XenServer::createNic(session, vm->getHandle(), device.c_str(), (xen_network)network.c_str()))
    {
        session.emit(XenObject::ERROR);
    }
}


void ControllerImpl::detachNic(VirtualInterface& vif)
{
    StringBuffer message;
    message.format(gettext("Do you wish to detach %s?"), vif.getDeviceName().c_str());
    if (!View::instance().askYesNo(Glib::ustring(message)))
    {
        return;
    }
    RefPtr<VirtualInterface> vifPtr(&vif, 1);
    schedule(sigc::bind<RefPtr<VirtualInterface> >(sigc::mem_fun(*this, &ControllerImpl::detachNicInBackground), vifPtr));
}


void ControllerImpl::detachNicInBackground(RefPtr<VirtualInterface> vif)
{
    RefPtr<VirtualMachine> vm = vif->getVm();
    if (!vm)
    {
        return;
    }
    XenObject::Busy busy(*vm);
    Session& session = vm->getSession();
    Session::Lock lock(session);
    vm->setDisplayStatus(gettext("Detaching NIC..."));
    if (!xen_vif_destroy(session, vif->getHandle()))
    {
        session.emit(XenObject::ERROR);
    }
}


void ControllerImpl::changeSnapshotName()
{
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedSnapshot();
    if (!vm || vm->isBusy())
    {
        return;
    }
    XenPtr<xen_vm_record> record = vm->getRecord();
    Glib::ustring label(record->name_label);
    Glib::ustring description(record->name_description);
    if (!View::instance().getName(gettext("Change VM Snapshot label/description"), label, description))
    {
        return;
    }
    Session& session = vm->getSession();
    Session::Lock lock(session);
    vm->setName(label.c_str(), description.c_str());
}


void ControllerImpl::snapshotVm()
{
    Trace trace(NULL, "ControllerImpl::snapshotVm");
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedVm();
    if (!vm || vm->isBusy())
    {
        trace.put(vm ? "Selected VM is busy." : "No selected VM.");
        return;
    }
    schedule(sigc::bind<RefPtr<VirtualMachine> >(sigc::mem_fun(*this, &ControllerImpl::snapshotVmInBackground), vm));
}


void ControllerImpl::snapshotVmInBackground(RefPtr<VirtualMachine> vm)
{
    Trace trace(NULL, "ControllerImpl::snapshotVmInBackground");
    XenObject::Busy busy(*vm);
    Session& session = vm->getSession();
    Session::Lock lock(session);
    vm->setDisplayStatus(gettext("Creating snapshot..."));
    if (!XenServer::createSnapshot(session, vm->getHandle()))
    {
        session.emit(XenObject::ERROR);
    }
}


void ControllerImpl::revertVm()
{
    Trace trace(NULL, "ControllerImpl::revertVm");
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedSnapshot();
    if (!vm  || vm->isBusy())
    {
        trace.put(vm ? "Selected snapshot is busy." : "No selected snapshot.");
        return;
    }
    XenPtr<xen_vm_record> record = vm->getRecord();
    Session& session = vm->getSession();
    RefPtr<VirtualMachine> src = session.getStore().getVm(record->snapshot_of);
    if (!src || src->isBusy())
    {
        trace.put(src ? "Selected snapshot source is busy." : "No selected snapshot source.");
        return;
    }
    schedule(sigc::bind<RefPtr<VirtualMachine> >(sigc::mem_fun(*this, &ControllerImpl::revertVmInBackground), vm));
}


void ControllerImpl::revertVmInBackground(RefPtr<VirtualMachine> vm)
{
    Trace trace(NULL, "ControllerImpl::revertVmInBackground");
    Session& session = vm->getSession();
    XenPtr<xen_vm_record> record = vm->getRecord();
    RefPtr<VirtualMachine> src = session.getStore().getVm(record->snapshot_of);
    XenObject::Busy busy(*src);
    Session::Lock lock(session);
    src->setDisplayStatus(gettext("Reverting..."));
    if (!xen_vm_revert(session, vm->getHandle()))
    {
        session.emit(XenObject::ERROR);
    }
}


void ControllerImpl::deleteSnapshot()
{
    Trace trace(NULL, "ControllerImpl::deleteSnapshot");
    RefPtr<VirtualMachine> vm = Model::instance().getSelectedSnapshot();
    if (!vm  || vm->isBusy())
    {
        trace.put(vm ? "Selected snapshot is busy." : "No selected snapshot.");
        return;
    }
    std::list<Glib::ustring> disks;
    if (!View::instance().getDisksToDelete(*vm, disks))
    {
        return;
    }
    schedule(sigc::bind<RefPtr<VirtualMachine>, std::list<Glib::ustring> >(sigc::mem_fun(*this, &ControllerImpl::deleteVmInBackground), vm, disks));
}
