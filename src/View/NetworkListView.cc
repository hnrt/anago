// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "NetworkListView.h"
#include "Model/Network.h"
#include "Model/Session.h"
#include "Model/XenObjectStore.h"


using namespace hnrt;


NetworkListView::NetworkListView(Session& session)
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


void NetworkListView::initStore(Session& session)
{
    std::list<std::pair<String, String> > a;
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
        std::pair<String, String> entry(String(record->name_label), (*ii)->getREFID());
        for (std::list<std::pair<String, String> >::iterator i = a.begin(); ; i++)
        {
            if (i == a.end())
            {
                a.push_back(entry);
                break;
            }
            else if (strcmp(entry.first, i->first) < 0)
            {
                a.insert(i, entry);
                break;
            }
        }
    }
    for (std::list<std::pair<String, String> >::const_iterator i = a.begin(); i != a.end(); i++)
    {
        Gtk::TreeModel::Row row = *_store->append();
        row[_record.colName] = Glib::ustring(i->first);
        row[_record.colValue] = i->second;
    }
}


int NetworkListView::getSelected(std::list<String>& list) const
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
