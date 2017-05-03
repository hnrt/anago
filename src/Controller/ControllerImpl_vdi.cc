// Copyright (C) 2012-2017 Hideaki Narita


#include "XenServer/VirtualDiskImage.h"
#include "ControllerImpl.h"


using namespace hnrt;


void ControllerImpl::resizeVdi(VirtualDiskImage& vdi)
{
    //TODO: IMPLEMENT
#if 0
    RefPtr<VirtualMachine> vm = vdi.getVm();
    if (vm)
    {
        XenPtr<xen_vm_record> record = vm->getRecord();
        if (record->power_state != XEN_VM_POWER_STATE_HALTED)
        {
            return;
        }
    }
    XenPtr<xen_vdi_record> record = vdi->getRecord();
    if (record->read_only)
    {
        return;
    }
    int64_t sizeCurrent = record->virtual_size;
    int64_t size;
    if (!View::instance().getSize(vdi, size))
    {
        return;
    }
    if (size == sizeCurrent)
    {
        return;
    }
    Session& session = vdi.getSession();
    Session::Lock lock(session);
    vdi.resize(size);
#endif
}


void ControllerImpl::removeVdi(VirtualDiskImage& vdi)
{
    //TODO: IMPLEMENT
#if 0
    if (!View::instance().confirmVdiToRemove(vdi))
    {
        return;
    }
    Session& session = vdi.getSession();
    Session::Lock lock(session);
    vdi.destroy();
#endif
}
