// Copyright (C) 2012-2018 Hideaki Narita


#include <libintl.h>
#include "XenServer/Session.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObjectStore.h"
#include "VirtualMachineTemplateListView.h"


using namespace hnrt;


VirtualMachineTemplateListView::VirtualMachineTemplateListView(const Session& session)
{
    _store = Gtk::ListStore::create(_record);
    initStore(session);

    set_model(_store);
    append_column(gettext("Name"), _record.colName);
    set_headers_visible(false);
    set_rules_hint(true);

    Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
    selection->set_mode(Gtk::SELECTION_SINGLE);
}


void VirtualMachineTemplateListView::initStore(const Session& session)
{
    std::list<RefPtr<VirtualMachine> > vmList;
    session.getStore().getList(vmList);
    for (std::list<RefPtr<VirtualMachine> >::iterator iter = vmList.begin(); iter != vmList.end(); iter++)
    {
        RefPtr<VirtualMachine> vm = *iter;
        XenPtr<xen_vm_record> vmRecord = vm->getRecord();
        if (!vmRecord || !vmRecord->is_a_template || vmRecord->is_a_snapshot)
        {
            continue;
        }
        Gtk::TreeModel::Row row;
        Gtk::TreeIter iter2 = _store->get_iter("0"); // point to first item
        while (iter2)
        {
            row = *iter2;
            Glib::ustring curName = row[_record.colName];
            if (curName > vm->getName())
            {
                row = *_store->insert(iter2);
                goto done;
            }
            iter2++;
        }
        row = *_store->append();
    done:
        row[_record.colName] = vm->getName();
        row[_record.colValue] = vm->getREFID();
    }
}


Glib::ustring VirtualMachineTemplateListView::getSelected() const
{
    Glib::ustring refid;
    Gtk::TreeIter iter = const_cast<VirtualMachineTemplateListView*>(this)->get_selection()->get_selected();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        refid = row[_record.colValue];
    }
    return refid;
}


Gtk::ScrolledWindow* VirtualMachineTemplateListView::createScrolledWindow()
{
    Gtk::ScrolledWindow* pW = new Gtk::ScrolledWindow();
    pW->add(*this);
    pW->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    pW->set_shadow_type(Gtk::SHADOW_IN);
    return pW;
}
