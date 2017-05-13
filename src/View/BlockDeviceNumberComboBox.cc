// Copyright (C) 2012-2017 Hideaki Narita


#include <list>
#include "XenServer/Session.h"
#include "XenServer/VirtualBlockDevice.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObjectStore.h"
#include "BlockDeviceNumberComboBox.h"


using namespace hnrt;


BlockDeviceNumberComboBox::BlockDeviceNumberComboBox(const VirtualMachine& vm)
{
    initStore(vm);
    Gtk::TreeIter iter0 = _store->get_iter("0"); // point to first item
    set_active(iter0);
}


BlockDeviceNumberComboBox::~BlockDeviceNumberComboBox()
{
}


static bool FindDev(const std::list<Glib::ustring>& devnos, const Glib::ustring& dev)
{
    for (std::list<Glib::ustring>::const_iterator iter = devnos.begin(); iter != devnos.end(); iter++)
    {
        if (*iter == dev)
        {
            return true;
        }
    }
    return false;
}


void BlockDeviceNumberComboBox::initStore(const VirtualMachine& vm)
{
    std::list<Glib::ustring> devnos;
    const Session& session = vm.getSession();
    XenPtr<xen_vm_record> vmRecord = vm.getRecord();
    for (size_t i = 0, n = vmRecord->vbds ? vmRecord->vbds->size : 0; i < n; i++)
    {
        RefPtr<VirtualBlockDevice> vbd = session.getStore().getVbd(vmRecord->vbds->contents[i]);
        if (vbd)
        {
            XenPtr<xen_vbd_record> vbdRecord = vbd->getRecord();
            if (vbdRecord)
            {
                devnos.push_back(Glib::ustring(vbdRecord->userdevice));
            }
        }
    }
    for (int i = 0, n = 0;; i++)
    {
        Glib::ustring dev = Glib::ustring::compose("%1", i);
        if (!FindDev(devnos, dev))
        {
            Gtk::TreeIter iter = _store->append();
            Gtk::TreeModel::Row row = *iter;
            row[_record.colDisplayName] = dev;
            if (++n >= 10)
            {
                break;
            }
        }
    }
}
