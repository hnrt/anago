// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Base/StringBuffer.h"
#include "XenServer/OpticalDevice.h"
#include "XenServer/OpticalDisk.h"
#include "XenServer/PhysicalBlockDevice.h"
#include "XenServer/Session.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/XenObjectStore.h"
#include "XenServer/XenPtr.h"
#include "CdImageListView.h"


using namespace hnrt;


CdImageListView::CdImageListView(const Session& session, bool addEmpty)
{
    _store = Gtk::ListStore::create(_record);
    initStore(session, addEmpty);

    set_model(_store);
    append_column(gettext("Name"), _record.colName);
    set_headers_visible(false);
    set_rules_hint(true);

    Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
    selection->set_mode(Gtk::SELECTION_SINGLE);
}


void CdImageListView::initStore(const Session& session, bool addEmpty)
{
    OpticalDiskList devList;
    OpticalDiskList isoList;
    std::list<RefPtr<StorageRepository> > srList;
    session.getStore().getList(srList);
    for (std::list<RefPtr<StorageRepository> >::const_iterator iter = srList.begin(); iter != srList.end(); iter++)
    {
        RefPtr<StorageRepository> sr = *iter;
        XenPtr<xen_sr_record> record = sr->getRecord();
        if (!record ||
            !record->pbds ||
            !record->pbds->size ||
            !record->vdis ||
            !record->vdis->size)
        {
            continue;
        }
        RefPtr<PhysicalBlockDevice> pbd = session.getStore().getPbd(record->pbds->contents[0]);
        if (!pbd)
        {
            continue;
        }
        XenPtr<xen_pbd_record> pbdRecord = pbd->getRecord();
        if (!pbdRecord || !pbdRecord->currently_attached)
        {
            continue;
        }
        for (size_t j = 0; j < record->vdis->size; j++)
        {
            RefPtr<VirtualDiskImage> vdi = session.getStore().getVdi(record->vdis->contents[j]);
            if (!vdi)
            {
                continue;
            }
            XenPtr<xen_vdi_record> vdiRecord = vdi->getRecord();
            StringBuffer name;
            switch (sr->getSubType())
            {
            case StorageRepository::DEV:
                name.format("%s (%s)", record->name_label, vdiRecord->name_label);
                devList.insert(vdi->getREFID(), Glib::ustring(name.str()));
                break;

            case StorageRepository::ISO:
                name.format("%s/%s", record->name_label, vdiRecord->name_label);
                isoList.insert(vdi->getREFID(), Glib::ustring(name.str()));
                break;

            default:
                break;
            }
        }
    }
    if (addEmpty)
    {
        Gtk::TreeModel::Row row = *_store->append();
        row[_record.colName] = Glib::ustring(gettext("(empty)"));
        row[_record.colValue] = Glib::ustring(NULLREFSTRING);
    }
    for (OpticalDiskList::ConstIter iter = devList.begin(); iter != devList.end(); iter++)
    {
        Gtk::TreeModel::Row row = *_store->append();
        row[_record.colName] = Glib::ustring(iter->name);
        row[_record.colValue] = iter->vdi;
    }
    for (OpticalDiskList::ConstIter iter = isoList.begin(); iter != isoList.end(); iter++)
    {
        Gtk::TreeModel::Row row = *_store->append();
        row[_record.colName] = Glib::ustring(iter->name);
        row[_record.colValue] = iter->vdi;
    }
}


Glib::ustring CdImageListView::getSelected() const
{
    Glib::ustring refid;
    Gtk::TreeIter iter = const_cast<CdImageListView*>(this)->get_selection()->get_selected();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        refid = row[_record.colValue];
    }
    return refid;
}


void CdImageListView::select(const Glib::ustring& refid)
{
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring refid2 = row[_record.colValue];
        if (refid2 == refid)
        {
            get_selection()->select(iter);
            return;
        }
        iter++;
    }
}


Gtk::ScrolledWindow* CdImageListView::createScrolledWindow()
{
    Gtk::ScrolledWindow* pW = new Gtk::ScrolledWindow();
    pW->add(*this);
    pW->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    pW->set_shadow_type(Gtk::SHADOW_IN);
    return pW;
}
