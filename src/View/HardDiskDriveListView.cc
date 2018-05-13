// Copyright (C) 2012-2018 Hideaki Narita


#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <stdexcept>
#include "Util/Util.h"
#include "XenServer/HardDiskDriveSpec.h"
#include "XenServer/Session.h"
#include "HardDiskDriveListView.h"


using namespace hnrt;


HardDiskDriveListView::HardDiskDriveListView(const Session& session)
    : _session(session)
{
    _store = Gtk::ListStore::create(_record);
    set_model(_store);
    append_column(gettext("Storage repository"), _record.colSr);
    append_column(gettext("Size"), _record.colSize);
    append_column(gettext("Label"), _record.colLabel);
    append_column(gettext("Description"), _record.colDesc);
    get_column(0)->set_resizable(true);
    get_column(0)->set_reorderable(false);
    get_column(1)->set_resizable(false);
    get_column(1)->set_reorderable(false);
    set_rules_hint(true);
    Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
    selection->set_mode(Gtk::SELECTION_SINGLE);
}


void HardDiskDriveListView::clear()
{
    _store->clear();
    _sigChanged.emit();
}


void HardDiskDriveListView::addValue(const HardDiskDriveSpec& spec)
{
    Gtk::TreeIter iter = _store->append();
    setValue(iter, spec);
}


void HardDiskDriveListView::removeValue(int index)
{
    int curIndex = 0;
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        if (curIndex == index)
        {
            removeValue(iter);
            return;
        }
        iter++;
        curIndex++;
    }
}


void HardDiskDriveListView::getValue(int index, HardDiskDriveSpec& spec) const
{
    int curIndex = 0;
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        if (curIndex == index)
        {
            Gtk::TreeModel::Row row = *iter;
            spec = row[_record.colSpec];
            return;
        }
        iter++;
        curIndex++;
    }
    throw std::runtime_error("HardDiskDriveListView::getValue: Index out of bounds.");
}


void HardDiskDriveListView::setValue(int index, const HardDiskDriveSpec& spec)
{
    int curIndex = 0;
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        if (curIndex == index)
        {
            setValue(iter, spec);
            return;
        }
        iter++;
        curIndex++;
    }
    throw std::runtime_error("HardDiskDriveListView::setValue: Index out of bounds.");
}


void HardDiskDriveListView::setValue(Gtk::TreeIter& iter, const HardDiskDriveSpec& spec)
{
    Gtk::TreeModel::Row row = *iter;
    row[_record.colSr] = spec.getSrName(_session);
    row[_record.colSize] = FormatSize(spec.size);
    row[_record.colLabel] = spec.label;
    row[_record.colDesc] = spec.description;
    row[_record.colSpec] = spec;
    _sigChanged.emit();
}


void HardDiskDriveListView::removeValue(Gtk::TreeIter& iter)
{
    _store->erase(iter);
    _sigChanged.emit();
}


int HardDiskDriveListView::getCount() const
{
    int count = 0;
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        count++;
        iter++;
    }
    return count;
}


int HardDiskDriveListView::getSelected()
{
    Gtk::TreeIter iterSelected = get_selection()->get_selected();
    if (iterSelected)
    {
        int index = 0;
        Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
        while (iter)
        {
            if (iter == iterSelected)
            {
                return index;
            }
            iter++;
            index++;
        }
    }
    return -1;
}


Gtk::ScrolledWindow* HardDiskDriveListView::createScrolledWindow()
{
    Gtk::ScrolledWindow* pW = new Gtk::ScrolledWindow();
    pW->add(*this);
    pW->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    pW->set_shadow_type(Gtk::SHADOW_IN);
    return pW;
}


HardDiskDriveListView::Record::Record()
{
    add(colSr);
    add(colSize);
    add(colLabel);
    add(colDesc);
    add(colSpec);
}
