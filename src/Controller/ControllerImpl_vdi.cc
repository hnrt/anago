// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Base/StringBuffer.h"
#include "View/View.h"
#include "XenServer/Session.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/VirtualMachine.h"
#include "ControllerImpl.h"


using namespace hnrt;


void ControllerImpl::changeVdiName(VirtualDiskImage& vdi)
{
    XenPtr<xen_vdi_record> record = vdi.getRecord();
    Glib::ustring label(record->name_label);
    Glib::ustring description(record->name_description);
    if (!View::instance().getName(gettext("Change VDI label/description"), label, description))
    {
        return;
    }
    Session& session = vdi.getSession();
    Session::Lock lock(session);
    vdi.setName(label.c_str(), description.c_str());
}


void ControllerImpl::resizeVdi(VirtualDiskImage& vdi)
{
    RefPtr<VirtualMachine> vm = vdi.getVm();
    if (vm)
    {
        XenPtr<xen_vm_record> record = vm->getRecord();
        if (record->power_state != XEN_VM_POWER_STATE_HALTED)
        {
            return;
        }
    }
    XenPtr<xen_vdi_record> record = vdi.getRecord();
    if (record->read_only)
    {
        return;
    }
    int64_t size = record->virtual_size;
    if (!View::instance().getSize(size))
    {
        return;
    }
    if (size == record->virtual_size)
    {
        return;
    }
    Session& session = vdi.getSession();
    Session::Lock lock(session);
    vdi.resize(size);
}


void ControllerImpl::removeVdi(VirtualDiskImage& vdi)
{
    XenPtr<xen_vdi_record> record = vdi.getRecord();
    StringBuffer message;
    message.format(gettext("Do you wish to delete the following virtual disk image?\n\n%1$s\n%2$s\n%3$'ld bytes"),
                   record->name_label,
                   record->name_description,
                   record->virtual_size);
    if (!View::instance().askYesNo(Glib::ustring(message.str())))
    {
        return;
    }
    Session& session = vdi.getSession();
    Session::Lock lock(session);
    vdi.destroy();
}
