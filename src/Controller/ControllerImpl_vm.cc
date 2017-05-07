// Copyright (C) 2012-2017 Hideaki Narita


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
    _tm.create(sigc::bind<RefPtr<Host>, VirtualMachineSpec>(sigc::mem_fun(*this, &ControllerImpl::addVmInBackground), host, spec), false, "AddVm");
}


void ControllerImpl::addVmInBackground(RefPtr<Host> host, VirtualMachineSpec spec)
{
    Trace trace("ControllerImpl::addVmInBackground");
    host->setBusy(true);
    try
    {
        Session& session = host->getSession();
        Session::Lock lock(session);
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
    host->setBusy(false);
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
        _tm.create(sigc::bind<RefPtr<VirtualMachine>, Glib::ustring>(sigc::mem_fun(*this, &ControllerImpl::cloneVmInBackground), vm, label), false, "CloneVm");
    }
    else
    {
        _tm.create(sigc::bind<RefPtr<VirtualMachine>, Glib::ustring, Glib::ustring>(sigc::mem_fun(*this, &ControllerImpl::copyVmInBackground), vm, label, srREFID), false, "CopyVm");
    }
}


void ControllerImpl::cloneVmInBackground(RefPtr<VirtualMachine> vm, Glib::ustring label)
{
    Trace trace("ControllerImpl::cloneVmInBackground");
    vm->setBusy(true);
    try
    {
        Session& session = vm->getSession();
        Session::Lock lock(session);
        vm->clone(label.c_str());
    }
    catch (...)
    {
        Logger::instance().error("%s: Unhandled exception caught.", trace.name().data());
    }
    vm->setBusy(false);
}


void ControllerImpl::copyVmInBackground(RefPtr<VirtualMachine> vm, Glib::ustring label, Glib::ustring srREFID)
{
    Trace trace("ControllerImpl::copyVmInBackground");
    vm->setBusy(true);
    try
    {
        Session& session = vm->getSession();
        Session::Lock lock(session);
        vm->copy(label.c_str(), (xen_sr)(char*)srREFID.c_str());
    }
    catch (...)
    {
        Logger::instance().error("%s: Unhandled exception caught.", trace.name().data());
    }
    vm->setBusy(false);
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
    _tm.create(sigc::bind<RefPtr<VirtualMachine>, std::list<Glib::ustring> >(sigc::mem_fun(*this, &ControllerImpl::deleteVmInBackground), vm, disks), false, "DeleteVm");
}


void ControllerImpl::deleteVmInBackground(RefPtr<VirtualMachine> vm, std::list<Glib::ustring> disks)
{
    Session& session = vm->getSession();
    Session::Lock lock(session);
    if (!vm->destroy())
    {
        return;
    }
    XenObjectStore& store = session.getStore();
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
            _tm.create(sigc::bind<RefPtr<VirtualMachine>, Glib::ustring, bool>(sigc::mem_fun(*this, &ControllerImpl::exportVmInBackground), vm, path, verify), false, "ExportVm");
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
            _tm.create(sigc::bind<RefPtr<Host>, Glib::ustring>(sigc::mem_fun(*this, &ControllerImpl::importVmInBackground), host, path), false, "ImportVm");
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
            _tm.create(sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &ControllerImpl::verifyVmInBackground), path), false, "VerifyVm");
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
    _tm.create(sigc::bind<RefPtr<VirtualMachine>, Glib::ustring, Glib::ustring>(sigc::mem_fun(*this, &ControllerImpl::attachHddInBackground), vm, userdevice, vdi), false, "AttachHdd");
}


void ControllerImpl::attachHddInBackground(RefPtr<VirtualMachine> vm, Glib::ustring userdevice, Glib::ustring vdi)
{
    XenObject::Busy busy(*vm);
    Session& session = vm->getSession();
    Session::Lock lock(session);
    if (!XenServer::attachHdd(session, vm->getHandle(), userdevice.c_str(), (xen_vdi)vdi.c_str()))
    {
        session.emit(XenObject::ERROR);
    }
}


void ControllerImpl::attachCd()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::attachNic()
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
