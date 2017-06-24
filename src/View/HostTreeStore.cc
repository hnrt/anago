// Copyright (C) 2012-2017 Hideaki Narita


#include "Logger/Trace.h"
#include "XenServer/Session.h"
#include "XenServer/XenObject.h"
#include "HostTreeStore.h"


using namespace hnrt;


Glib::RefPtr<HostTreeStore> HostTreeStore::create()
{
    return Glib::RefPtr<HostTreeStore>(new HostTreeStore);
}


HostTreeStore::HostTreeStore()
{
    set_column_types(_record);
}


bool HostTreeStore::row_draggable_vfunc(const Gtk::TreeModel::Path& path) const
{
    HostTreeStore* pThis = const_cast<HostTreeStore*>(this);
    Gtk::TreeIter iter = pThis->get_iter(path);
    Gtk::TreeModel::Row row = *iter;
    if (!row)
    {
        return false;
    }
    RefPtr<XenObject> object = row[_record.colXenObject];
    if (!object || object->getType() != XenObject::HOST)
    {
        return false;
    }
    return Gtk::TreeStore::row_draggable_vfunc(path);
}


bool HostTreeStore::row_drop_possible_vfunc(const Gtk::TreeModel::Path& destPath, const Gtk::SelectionData& selectionData) const
{
    Trace trace(this, "HostTreeStore::row_drop_possible_vfunc");

    Glib::RefPtr<Gtk::TreeModel> srcModel;
    Gtk::TreePath srcPath;
    if (Gtk::TreePath::get_from_selection_data(selectionData, srcModel, srcPath))
    {
        Gtk::TreeIter iter = srcModel->get_iter(srcPath);
        Gtk::TreeModel::Row row = *iter;
        RefPtr<XenObject> srcObject = row[_record.colXenObject];
        switch (srcObject->getType())
        {
        case XenObject::HOST:
        {
            HostTreeStore* pThis = const_cast<HostTreeStore*>(this);
            Gtk::TreeIter dstIter = pThis->get_iter(destPath);
            Gtk::TreeModel::Row dstRow = *dstIter;
            if (!dstRow)
            {
                break;
            }
            RefPtr<XenObject> dstObject = dstRow[_record.colXenObject];
            if (dstObject && dstObject->getType() == XenObject::HOST)
            {
                trace.put("dest=%s %s", destPath.to_string().c_str(), dstObject->getSession().getConnectSpec().displayname.c_str());
                return true;
            }
            break;
        }
        default:
            trace.put("ObjectType=%d", srcObject->getType());
            break;
        }
    }
    return false;
}


HostTreeStore::Record::Record()
{
    add(colPix);
    add(colKey);
    add(colVal);
    add(colXenObject);
}
