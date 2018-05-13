// Copyright (C) 2012-2018 Hideaki Narita


#include <libintl.h>
#include "XenServer/Network.h"
#include "XenServer/Session.h"
#include "XenServer/XenObjectStore.h"
#include "NetworkListView.h"


using namespace hnrt;


NetworkListView::NetworkListView(const Session& session)
{
    _store = Gtk::ListStore::create(_record);
    initStore(session);

    set_model(_store);
    append_column(gettext("Name"), _record.colName);
    set_headers_visible(false);
    set_rules_hint(true);

    Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
    selection->set_mode(Gtk::SELECTION_MULTIPLE);
}


void NetworkListView::initStore(const Session& session)
{
    std::list<std::pair<Glib::ustring, Glib::ustring> > a;
    std::list<RefPtr<Network> > nwList;
    session.getStore().getList(nwList);
    for (std::list<RefPtr<Network> >::const_iterator ii = nwList.begin(); ii != nwList.end(); ii++)
    {
        XenPtr<xen_network_record> record = (*ii)->getRecord();
        if (!record->pifs || !record->pifs->size)
        {
            // This check serves to exclude "Host internal management network".
            continue;
        }
        std::pair<Glib::ustring, Glib::ustring> entry(Glib::ustring(record->name_label), (*ii)->getREFID());
        for (std::list<std::pair<Glib::ustring, Glib::ustring> >::iterator i = a.begin(); ; i++)
        {
            if (i == a.end())
            {
                a.push_back(entry);
                break;
            }
            else if (entry.first < i->first)
            {
                a.insert(i, entry);
                break;
            }
        }
    }
    for (std::list<std::pair<Glib::ustring, Glib::ustring> >::const_iterator i = a.begin(); i != a.end(); i++)
    {
        Gtk::TreeModel::Row row = *_store->append();
        row[_record.colName] = i->first;
        row[_record.colValue] = i->second;
    }
}


int NetworkListView::getSelected(std::list<Glib::ustring>& list) const
{
    int count = 0;
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    Glib::RefPtr<Gtk::TreeSelection> selection = const_cast<NetworkListView*>(this)->get_selection();
    while (iter)
    {
        if (selection->is_selected(iter))
        {
            Gtk::TreeModel::Row row = *iter;
            list.push_back(row[_record.colValue]);
            count++;
        }
        iter++;
    }
    return count;
}


Gtk::ScrolledWindow* NetworkListView::createScrolledWindow()
{
    Gtk::ScrolledWindow* pW = new Gtk::ScrolledWindow();
    pW->add(*this);
    pW->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    pW->set_shadow_type(Gtk::SHADOW_IN);
    return pW;
}
