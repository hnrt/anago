// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <string.h>
#include "Base/StringBuffer.h"
#include "XenServer/Session.h"
#include "XenServer/VirtualBlockDevice.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObjectStore.h"
#include "XenServer/XenPtr.h"
#include "VirtualDiskImageListView.h"


using namespace hnrt;


VirtualDiskImageListView::VirtualDiskImageListView()
    : _pSession(NULL)
{
    _store = Gtk::ListStore::create(_record);
    set_model(_store);
    append_column(gettext("UUID"), _record.colUUID);
    append_column(gettext("Name"), _record.colName);
    append_column(gettext("Description"), _record.colDesc);
    append_column(gettext("Type"), _record.colType);
    append_column(gettext("Virtual size"), _record.colSize);
    append_column(gettext("Physically used"), _record.colUsed);
    append_column(gettext("Location"), _record.colLocation);
    append_column(gettext("Sharable"), _record.colSharable);
    append_column(gettext("Read only"), _record.colReadOnly);
    append_column(gettext("Managed"), _record.colManaged);
    append_column(gettext("Missing"), _record.colMissing);
    append_column(gettext("Snapshot"), _record.colSnapshot);
    append_column(gettext("Virtual machine"), _record.colVm);
    append_column(gettext("Parent"), _record.colParent);
    for (int i = 0; i < 13; i++)
    {
        get_column(i)->set_reorderable();
        get_column(i)->set_resizable(true);
    }
    set_rules_hint(true);
    get_selection()->set_mode(Gtk::SELECTION_SINGLE);
}


void VirtualDiskImageListView::update(const Session& session, const xen_vdi_record_opt_set* vdis, int flags)
{
    _pSession = &session;

    Gtk::TreeModel::Row row;
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        row = *iter;
        row[_record.colUpdated] = false;
        iter++;
    }

    if (vdis)
    {
        for (size_t i = 0; i < vdis->size; i++)
        {
            RefPtr<VirtualDiskImage> vdi = session.getStore().getVdi(vdis->contents[i]);
            if (!vdi)
            {
                continue;
            }
            XenPtr<xen_vdi_record> vdiRecord = vdi->getRecord();
            if (!vdiRecord)
            {
                continue;
            }
            Glib::ustring vmUUID, vmName;
            if (vdiRecord->vbds && vdiRecord->vbds->size)
            {
                RefPtr<VirtualBlockDevice> vbd = session.getStore().getVbd(vdiRecord->vbds->contents[0]);
                if (vbd)
                {
                    XenPtr<xen_vbd_record> vbdRecord = vbd->getRecord();
                    if (vbdRecord)
                    {
                        if (vbdRecord->type == XEN_VBD_TYPE_DISK)
                        {
                            RefPtr<VirtualMachine> vm = session.getStore().getVm(vbdRecord->vm);
                            if (vm)
                            {
                                XenPtr<xen_vm_record> vmRecord = vm->getRecord();
                                if (vmRecord)
                                {
                                    vmUUID = vmRecord->uuid;
                                    vmName = vmRecord->name_label;
                                }
                            }
                        }
                    }
                }
            }
            if ((flags & ATTACHABLE_ONLY))
            {
                if (vmUUID.bytes() || vdiRecord->is_a_snapshot || !vdiRecord->managed)
                {
                    continue;
                }
            }
            bool found = false;
            iter = _store->get_iter("0"); // point to first item
            while (iter)
            {
                row = *iter;
                if (static_cast<Glib::ustring>(row[_record.colUUID]) == vdiRecord->uuid)
                {
                    found = true;
                    break;
                }
                iter++;
            }
            if (!found)
            {
                iter = _store->append();
                row = *iter;
                row[_record.colREFID] = vdi->getREFID();
                row[_record.colUUID] = Glib::ustring(vdiRecord->uuid);
            }
            row[_record.colName] = Glib::ustring(vdiRecord->name_label);
            row[_record.colDesc] = Glib::ustring(vdiRecord->name_description);
            row[_record.colType] = Glib::ustring(xen_vdi_type_to_string(vdiRecord->type));
            row[_record.colSize] = Glib::ustring(StringBuffer().format("%'ld", vdiRecord->virtual_size));
            row[_record.colUsed] = Glib::ustring(StringBuffer().format("%'ld", vdiRecord->physical_utilisation));
            row[_record.colLocation] = Glib::ustring(vdiRecord->location);
            row[_record.colSharable] = Glib::ustring(vdiRecord->sharable ? "true" : "false");
            row[_record.colReadOnly] = Glib::ustring(vdiRecord->read_only ? "true" : "false");
            row[_record.colManaged] = Glib::ustring(vdiRecord->managed ? "true" : "false");
            row[_record.colMissing] = Glib::ustring(vdiRecord->missing ? "true" : "false");
            row[_record.colSnapshot] = Glib::ustring(vdiRecord->is_a_snapshot ? "true" : "false");
            if (vmName.empty())
            {
                row[_record.colVm] = vmUUID;
            }
            else
            {
                row[_record.colVm] = Glib::ustring::compose("%1 (%2)", vmUUID, vmName);
            }
            const char *parent = XenServer::getParent(vdiRecord);
            row[_record.colParent] = Glib::ustring(parent ? parent : "");
            row[_record.colUpdated] = true;
        }
    }

    iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        row = *iter;
        if (row[_record.colUpdated] == false)
        {
            iter = _store->erase(iter);
        }
        else
        {
            iter++;
        }
    }
}


bool VirtualDiskImageListView::on_button_press_event(GdkEventButton* event)
{
    bool retval = Gtk::TreeView::on_button_press_event(event);

    if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
    {
        Gtk::TreeIter iter = get_selection()->get_selected();
        if (iter)
        {
            Gtk::TreeModel::Row row = *iter;
            Glib::ustring refid = row[_record.colREFID];
            _menu.popup(event->button, event->time, _pSession->getStore().getVdi(refid));
        }
        // The event has been handled.
        return true;
    }

    return retval;
}


Glib::ustring VirtualDiskImageListView::getSelected()
{
    Glib::ustring refid;
    Gtk::TreeIter iter = get_selection()->get_selected();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        refid = row[_record.colREFID];
    }
    return refid;
}


Gtk::ScrolledWindow* VirtualDiskImageListView::createScrolledWindow()
{
    Gtk::ScrolledWindow* pW = new Gtk::ScrolledWindow();
    pW->add(*this);
    pW->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    pW->set_shadow_type(Gtk::SHADOW_IN);
    return pW;
}
