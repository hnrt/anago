// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Base/StringBuffer.h"
#include "Util/Util.h"
#include "XenServer/Session.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/XenObjectStore.h"
#include "StorageRepositoryComboBox.h"


using namespace hnrt;


StorageRepositoryComboBox::StorageRepositoryComboBox(Session& session)
    : _session(session)
{
    _session.incRef();

    _store = Gtk::ListStore::create(_record);
    initStore();

    set_model(_store);
    pack_start(_record.colDisplayName, true);
}


StorageRepositoryComboBox::~StorageRepositoryComboBox()
{
    _session.decRef();
}


void StorageRepositoryComboBox::initStore()
{
    std::list<RefPtr<StorageRepository> > list;
    _session.getStore().getList(list);
    for (std::list<RefPtr<StorageRepository> >::iterator iter = list.begin(); iter != list.end(); iter++)
    {
        RefPtr<StorageRepository> sr = *iter;
        if (sr && sr->getSubType() == StorageRepository::USR)
        {
            add(*sr);
        }
    }
}


void StorageRepositoryComboBox::add(StorageRepository& sr)
{
    Glib::ustring name = sr.getName();
    Glib::ustring refid = sr.getREFID();
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring curName = row[_record.colName];
        if (curName > name)
        {
            iter = _store->insert(iter);
            goto done;
        }
        iter++;
    }
    iter = _store->append();
done:
    {
        bool bDefault = sr.isDefault();
        XenPtr<xen_sr_record> srRecord = sr.getRecord();
        StringBuffer displayName;
        displayName.format(gettext("%s %s (%ld%% used, %s free)%s"),
                           sr.getName().c_str(),
                           FormatSize(srRecord->physical_size).c_str(),
                           (100 * srRecord->physical_utilisation) / srRecord->physical_size,
                           FormatSize(srRecord->physical_size - srRecord->physical_utilisation).c_str(),
                           bDefault ? gettext(" [default]") : "");
        Gtk::TreeModel::Row row = *iter;
        row[_record.colDisplayName] = Glib::ustring(displayName.str());
        row[_record.colName] = name;
        row[_record.colValue] = refid;
    }
}


void StorageRepositoryComboBox::select(const Glib::ustring& refid)
{
    Gtk::TreeIter iter0 = _store->get_iter("0"); // point to first item
    Gtk::TreeIter iter = iter0;
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring curREFID = row[_record.colValue];
        if (curREFID == refid)
        {
            // Found!
            set_active(iter);
            return;
        }
        iter++;
    }
    selectDefault();
}


void StorageRepositoryComboBox::selectDefault()
{
    Gtk::TreeIter iter0 = _store->get_iter("0"); // point to first item
    Gtk::TreeIter iter = iter0;
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring refid = row[_record.colValue];
        RefPtr<StorageRepository> sr = _session.getStore().getSr(refid);
        if (sr->isDefault())
        {
            // Found!
            set_active(iter);
            return;
        }
        iter++;
    }
    set_active(iter0);
}


Glib::ustring StorageRepositoryComboBox::getSelected()
{
    Glib::ustring refid;
    Gtk::TreeIter iter = get_active();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        refid = row[_record.colValue];
    }
    return refid;
}
