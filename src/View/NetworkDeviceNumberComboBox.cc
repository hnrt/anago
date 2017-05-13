// Copyright (C) 2012-2017 Hideaki Narita


#include <list>
#include "XenServer/Session.h"
#include "XenServer/VirtualInterface.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObjectStore.h"
#include "XenServer/XenPtr.h"
#include "NetworkDeviceNumberComboBox.h"


using namespace hnrt;


NetworkDeviceNumberComboBox::NetworkDeviceNumberComboBox(const VirtualMachine& vm)
{
    initStore(vm);
    Gtk::TreeIter iter0 = _store->get_iter("0"); // point to first item
    set_active(iter0);
}


NetworkDeviceNumberComboBox::~NetworkDeviceNumberComboBox()
{
}


static bool Find(const std::list<Glib::ustring>& list, const Glib::ustring& dev)
{
    for (std::list<Glib::ustring>::const_iterator iter = list.begin(); iter != list.end(); iter++)
    {
        if (*iter == dev)
        {
            return true;
        }
    }
    return false;
}


void NetworkDeviceNumberComboBox::initStore(const VirtualMachine& vm)
{
    std::list<Glib::ustring> devs;
    const Session& session = vm.getSession();
    XenPtr<xen_vm_record> vmRecord = vm.getRecord();
    for (size_t i = 0, n = vmRecord->vifs ? vmRecord->vifs->size : 0; i < n; i++)
    {
        RefPtr<VirtualInterface> vif = session.getStore().getVif(vmRecord->vifs->contents[i]);
        if (!vif)
        {
            continue;
        }
        XenPtr<xen_vif_record> vifRecord = vif->getRecord();
        if (vifRecord)
        {
            devs.push_back(Glib::ustring(vifRecord->device));
        }
    }
    for (int i = 0, n = 0;; i++)
    {
        Glib::ustring dev = Glib::ustring::compose("%1", i);
        if (!Find(devs, dev))
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
