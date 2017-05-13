// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "XenServer/OpticalDevice.h"
#include "XenServer/Session.h"
#include "XenServer/VirtualBlockDevice.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObjectStore.h"
#include "XenServer/XenPtr.h"
#include "CdDeviceComboBox.h"


using namespace hnrt;


CdDeviceComboBox::CdDeviceComboBox(const VirtualMachine& vm)
{
    _store = Gtk::ListStore::create(_record);
    initStore(vm);

    set_model(_store);
    pack_start(_record.colName, true);

    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    if (iter)
    {
        set_active(iter);
    }
    else
    {
        unset_active();
    }
}


void CdDeviceComboBox::initStore(const VirtualMachine& vm)
{
    OpticalDeviceList devList;
    const Session& session = vm.getSession();
    const XenPtr<xen_vm_record> record = vm.getRecord();
    for (size_t i = 0; i < record->vbds->size; i++)
    {
        RefPtr<VirtualBlockDevice> vbd = session.getStore().getVbd(record->vbds->contents[i]);
        if (!vbd)
        {
            continue;
        }
        XenPtr<xen_vbd_record> vbdRecord = vbd->getRecord();
        if (vbdRecord->type != XEN_VBD_TYPE_CD)
        {
            continue;
        }
        Glib::ustring vdiREFID;
        RefPtr<VirtualDiskImage> vdi = session.getStore().getVdi(vbdRecord->vdi);
        if (vdi)
        {
            vdiREFID = vdi->getREFID();
        }
        else
        {
            vdiREFID = NULLREFSTRING;
        }
        devList.insert(vbd->getREFID(), vbd->getDeviceName(), vdiREFID);
    }
    for (OpticalDeviceList::Iter iter = devList.begin(); iter != devList.end(); iter++)
    {
        Gtk::TreeModel::Row row = *_store->append();
        row[_record.colREFID] = iter->vbd;
        row[_record.colName] = iter->name;
        row[_record.colImageREFID] = iter->vdi;
    }
}


void CdDeviceComboBox::select(const Glib::ustring& refid)
{
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring curREFID = row[_record.colREFID];
        if (curREFID == refid)
        {
            // Found!
            set_active(iter);
            return;
        }
        iter++;
    }
    // As no entry of the given REFID exists in the _store, the first element will be selected.
    // If there is no element, this combo box will be in the unselected state.
    set_active(_store->get_iter("0"));
}


Glib::ustring CdDeviceComboBox::getSelected() const
{
    Glib::ustring refid;
    Gtk::TreeIter iter = get_active();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        refid = row[_record.colREFID];
    }
    return refid;
}


Glib::ustring CdDeviceComboBox::getSelectedImage() const
{
    Glib::ustring refid;
    Gtk::TreeIter iter = get_active();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        refid = row[_record.colImageREFID];
    }
    return refid;
}
