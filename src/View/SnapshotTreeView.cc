// Copyright (C) 2012-2017 Hideaki Narita


#include <time.h>
#include <libintl.h>
#include "Base/StringBuffer.h"
#include "XenServer/Session.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObjectStore.h"
#include "PixStore.h"
#include "SnapshotTreeView.h"


using namespace hnrt;


SnapshotTreeView::SnapshotTreeView()
{
    _store = Gtk::TreeStore::create(_record);
    set_model(_store);
    set_headers_visible(false);
    append_column(gettext("Pix"), _record.colPix);
    append_column(gettext("Name"), _record.colName);
    set_rules_hint(true);
    Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
    selection->set_mode(Gtk::SELECTION_SINGLE);
}


void SnapshotTreeView::clear()
{
    _store->clear();
}


void SnapshotTreeView::set(const RefPtr<VirtualMachine>& vm)
{
    clear();

    _vm = vm;

    XenPtr<xen_vm_record> record = _vm->getRecord();

    xen_vm_record_opt_set* snapshots = record->snapshots;
    if (!snapshots)
    {
        return;
    }

    Session& session = _vm->getSession();
    XenObjectStore& xoStore = session.getStore();

    for (size_t i = 0; i < snapshots->size; i++)
    {
        RefPtr<VirtualMachine> snapshotVm = xoStore.getVm(record->snapshots->contents[i]);
        if (snapshotVm)
        {
            add(snapshotVm);
        }
    }

    add(_vm);
}


void SnapshotTreeView::add(const RefPtr<VirtualMachine>& vm)
{
    XenPtr<xen_vm_record> record = vm->getRecord();
    RefPtr<VirtualMachine> parent = vm->getSession().getStore().getVm(record->parent);
    if (parent)
    {
        add(parent);
        Glib::ustring refid = parent->getREFID();
        Gtk::TreeIter iter = find(refid, _store->get_iter("0"));
        if (iter)
        {
            Gtk::TreeIter iter2 = findPositionOfInsertion(vm, iter->children().begin());
            if (iter2)
            {
                iter2 = _store->insert(iter2);
            }
            else
            {
                iter2 = _store->append(iter->children());
            }
            set(*iter2, vm);
            expand_row(_store->get_path(iter), true);
        }
    }
    else
    {
        Gtk::TreeIter iter = findPositionOfInsertion(vm, _store->get_iter("0"));
        if (iter)
        {
            iter = _store->insert(iter);
        }
        else
        {
            iter = _store->append();
        }
        set(*iter, vm);
    }
}


Gtk::TreeIter SnapshotTreeView::find(const Glib::ustring& refid, Gtk::TreeIter iter)
{
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring next = row[_record.colREFID];
        if (next == refid)
        {
            break;
        }
        Gtk::TreeIter iter2 = find(refid, row.children().begin());
        if (iter2)
        {
            iter = iter2;
            break;
        }
        iter++;
    }
    return iter;
}


Gtk::TreeIter SnapshotTreeView::findPositionOfInsertion(const RefPtr<VirtualMachine>& vm, Gtk::TreeIter iter)
{
    XenPtr<xen_vm_record> record = vm->getRecord();
    if (!record->is_a_snapshot)
    {
        return iter;
    }
    unsigned long snapshotTime = (unsigned long)record->snapshot_time;
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        if (snapshotTime < row[_record.colTime])
        {
            return iter;
        }
        iter++;
    }
    return iter;
}


void SnapshotTreeView::set(const Gtk::TreeModel::Row& row, const RefPtr<VirtualMachine>& vm)
{
    XenPtr<xen_vm_record> record = vm->getRecord();
    StringBuffer name;
    if (record->is_a_snapshot)
    {
        char ts[64] = { 0 };
        struct tm tmVal = { 0 };
        time_t snapshotTime = record->snapshot_time - timezone;
        localtime_r(&snapshotTime, &tmVal);
        strftime(ts, sizeof(ts), "%F %T", &tmVal);
        name.format("%s\n%s\n%s",
                    record->name_label ? record->name_label : "",
                    record->name_description ? record->name_description : "",
                    ts);
    }
    else
    {
        name.format("%s\n%s\n%s",
                    record->name_label ? record->name_label : "",
                    record->name_description ? record->name_description : "",
                    gettext("Current virtual machine"));
    }
    row[_record.colPix] = record->is_a_snapshot ? PixStore::instance().getComputer() : PixStore::instance().get(*vm);
    row[_record.colName] = Glib::ustring(name);
    row[_record.colREFID] = vm->getREFID();
    row[_record.colTime] = (unsigned long)record->snapshot_time;
}


bool SnapshotTreeView::on_button_press_event(GdkEventButton* event)
{
    bool retval = Gtk::TreeView::on_button_press_event(event);

    if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
    {
        Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
        Gtk::TreeIter iter = selection->get_selected();
        if (iter)
        {
            Gtk::TreeModel::Row row = *iter;
            Glib::ustring refid = row[_record.colREFID];
            RefPtr<VirtualMachine> vm = _vm->getSession().getStore().getVm(refid);
            if (vm)
            {
                _menu.popup(event->button, event->time, vm);
                // The event has been handled.
                return true;
            }
        }
    }

    return retval;
}
