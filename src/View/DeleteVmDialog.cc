// Copyright (C) 2012-2017 Hideaki Narita


#include <string.h>
#include <libintl.h>
#include "Model/Model.h"
#include "Util/Util.h"
#include "XenServer/Session.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/VirtualBlockDevice.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObjectStore.h"
#include "XenServer/XenPtr.h"
#include "DeleteVmDialog.h"


using namespace hnrt;


DeleteVmDialog::DeleteVmDialog(Gtk::Window& parent, const VirtualMachine& vm)
    : Gtk::Dialog(gettext("Delete VM"), parent)
{
    set_default_size(300, 240);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::DELETE, Gtk::RESPONSE_APPLY);

    bool snapshot = vm.getRecord()->is_a_snapshot;
    if (snapshot)
    {
        set_title(gettext("Delete VM snapshot"));
    }

    Gtk::VBox* box = get_vbox();
    box->set_spacing(6);

    Glib::ustring text = Glib::ustring::compose(
        snapshot ?
        gettext("The virtual machine snapshot named \"%1\" will be deleted. The selected virtual disks listed below will also be deleted.") :
        gettext("The virtual machine named \"%1\" will be deleted. The selected virtual disks listed below will also be deleted."),
        vm.getName());
    _descLabel.set_text(text);
    _descLabel.set_line_wrap(true);
    _descLabel.set_alignment(0.0, 0.5); // h=left, v=center
    box->pack_start(_descLabel, Gtk::PACK_SHRINK);

    _vdiSw.add(_vdiLv);
    _vdiSw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    box->pack_start(_vdiSw);

    _store = Gtk::ListStore::create(_record);
    initStore(vm);

    _vdiLv.set_model(_store);
    Gtk::CellRendererToggle* cellDelete = Gtk::manage(new Gtk::CellRendererToggle);
    _vdiLv.append_column(gettext("Delete"), *cellDelete);
    _vdiLv.get_column(0)->add_attribute(cellDelete->property_active(), _record.colDelete);
    cellDelete->signal_toggled().connect(sigc::mem_fun(*this, &DeleteVmDialog::onToggled));
    _vdiLv.append_column(gettext("Name"), _record.colName);
    _vdiLv.append_column(gettext("Size"), _record.colSize);
    _vdiLv.append_column(gettext("SR"), _record.colSr);
    _vdiLv.append_column(gettext("Snapshot"), _record.colSnapshot);
    Glib::RefPtr<Gtk::TreeSelection> selection = _vdiLv.get_selection();
    selection->set_mode(Gtk::SELECTION_NONE);

    show_all_children();
}


void DeleteVmDialog::onToggled(const Glib::ustring& path)
{
    Gtk::TreeModel::Row row = *_store->get_iter(Gtk::TreeModel::Path(path));
    row[_record.colDelete] = !row[_record.colDelete];
}


void DeleteVmDialog::initStore(const VirtualMachine& vm)
{
    const Session& session = vm.getSession();
    XenPtr<xen_vm_record> vmRecord = vm.getRecord();
    if (!vmRecord || !vmRecord->vbds)
    {
        return;
    }

    for (size_t i = 0; i < vmRecord->vbds->size; i++)
    {
        RefPtr<VirtualBlockDevice> vbd = session.getStore().getVbd(vmRecord->vbds->contents[i]);
        if (!vbd)
        {
            continue;
        }
        XenPtr<xen_vbd_record> vbdRecord = vbd->getRecord();
        if (!vbdRecord)
        {
            continue;
        }
        if (vbdRecord->type != XEN_VBD_TYPE_DISK)
        {
            continue;
        }
        RefPtr<VirtualDiskImage> vdi = session.getStore().getVdi(vbdRecord->vdi);
        if (!vdi)
        {
            continue;
        }
        XenPtr<xen_vdi_record> vdiRecord = vdi->getRecord();
        if (!vdiRecord)
        {
            continue;
        }
        RefPtr<StorageRepository> sr = session.getStore().getSr(vdiRecord->sr);
        if (!sr)
        {
            continue;
        }
        Glib::ustring srName;
        XenPtr<xen_sr_record> srRecord = sr->getRecord();
        if (srRecord)
        {
            srName = srRecord->name_label;
        }
        else
        {
            srName = "?";
        }
        StringBuffer displaySize;
        FormatSize(displaySize, vdiRecord->virtual_size);
        const char* snapshot = vmRecord->is_a_snapshot ? gettext("Yes") : gettext("No");
        add(vdi->getREFID().c_str(), vdiRecord->name_label, displaySize.str(), srName.c_str(), snapshot);
    }
}


void DeleteVmDialog::add(const char* vdi, const char* name, const char* size, const char* srName, const char* snapshot)
{
    Gtk::TreeModel::Row row;
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        row = *iter;
        Glib::ustring curName = row[_record.colName];
        if (curName > name)
        {
            row = *_store->insert(iter);
            goto done;
        }
        iter++;
    }
    row = *_store->append();
done:
    row[_record.colREFID] = Glib::ustring(vdi);
    row[_record.colName] = Glib::ustring(name);
    row[_record.colSize] = Glib::ustring(size);
    row[_record.colSr] = Glib::ustring(srName);
    row[_record.colSnapshot] = Glib::ustring(snapshot);
    row[_record.colDelete] = true;
}


int DeleteVmDialog::getDisks(std::list<Glib::ustring>& disks) const
{
    int count = 0;
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        if (row[_record.colDelete])
        {
            disks.push_back(row[_record.colREFID]);
            count++;
        }
        iter++;
    }
    return count;
}
