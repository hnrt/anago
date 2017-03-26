// Copyright (C) 2012-2017 Hideaki Narita


#include <stdio.h>
#include <libintl.h>
#include <stdexcept>
#include "Controller/Controller.h"
#include "Controller/SignalManager.h"
#include "Logger/Trace.h"
#include "XenServer/Host.h"
#include "XenServer/Network.h"
#include "XenServer/Session.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObjectStore.h"
#include "PixStore.h"
#include "HostTreeStore.h"
#include "HostTreeView.h"


using namespace hnrt;


HostTreeView::HostTreeView()
{
    _store = HostTreeStore::create();
    set_model(_store);
    set_headers_visible(false);
    set_reorderable();
    append_column(gettext("Pix"), _store->record().colPix);
    append_column(gettext("ID"), _store->record().colKey);
    append_column(gettext("Contents"), _store->record().colVal);
    Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
    selection->set_mode(Gtk::SELECTION_MULTIPLE);
    SignalManager::instance().xenObjectSignal(XenObject::CREATED).connect(sigc::mem_fun(*this, &HostTreeView::onObjectCreated));
}


HostTreeView::~HostTreeView()
{
}


void HostTreeView::clear()
{
    _store.clear();
}


void HostTreeView::onObjectCreated(RefPtr<XenObject> object, int what)
{
    Trace trace("HostTreeView::onObjectCreated");
    AddObject add = getAdd(object);
    if ((this->*add)(object))
    {
        trace.put("add=true");
        SignalManager::instance().xenObjectSignal(*object).connect(sigc::mem_fun(*this, &HostTreeView::onObjectUpdated));
        _signalNodeCreated.emit(object);
    }
}


void HostTreeView::onObjectUpdated(RefPtr<XenObject> object, int what)
{
    if (what == XenObject::DESTROYED)
    {
        remove(*object);
        return;
    }
    else
    {
        UpdateObject update = getUpdate(object);
        (this->*update)(object, what);
    }
}


HostTreeView::AddObject HostTreeView::getAdd(const RefPtr<XenObject>& object)
{
    switch (object->getType())
    {
    case XenObject::HOST:
        return &HostTreeView::addHost;
    case XenObject::VM:
        return &HostTreeView::addVm;
    case XenObject::SR:
        return &HostTreeView::addObject;
    case XenObject::NETWORK:
        return &HostTreeView::addObject;
    default:
        return &HostTreeView::addNothing;
    }
}


HostTreeView::UpdateObject HostTreeView::getUpdate(const RefPtr<XenObject>& object)
{
    switch (object->getType())
    {
    case XenObject::HOST:
        return &HostTreeView::updateHost;
    case XenObject::VM:
        return &HostTreeView::updateObject;
    case XenObject::SR:
        return &HostTreeView::updateObject;
    case XenObject::NETWORK:
        return &HostTreeView::updateObject;
    default:
        return &HostTreeView::updateNothing;
    }
}


bool HostTreeView::addHost(RefPtr<XenObject> object)
{
    Gtk::TreeIter iter = findHost(*object, true);
    Gtk::TreeModel::Row row = *iter;
    row[_store->record().colPix] = PixStore::instance().get(*object);
    row[_store->record().colKey] = object->getSession().getConnectSpec().displayname;
    row[_store->record().colVal] = object->getDisplayStatus();
    row[_store->record().colXenObject] = object;
    return true;
}


void HostTreeView::updateHost(RefPtr<XenObject> object, int what)
{
    Gtk::TreeIter iter = findHost(*object, false);
    if (!iter)
    {
        return;
    }
    Gtk::TreeModel::Row row = *iter;
    switch (what)
    {
    case XenObject::NAME_UPDATED:
        row[_store->record().colKey] = object->getSession().getConnectSpec().displayname;
        break;
    case XenObject::BUSY_SET:
    case XenObject::BUSY_RESET:
        row[_store->record().colPix] = PixStore::instance().get(*object);
        break;
    case XenObject::CONNECTED:
        row[_store->record().colPix] = PixStore::instance().get(*object);
        break;
    case XenObject::DISCONNECTED:
    {
        row[_store->record().colPix] = PixStore::instance().get(*object);
        iter = row.children().begin();
        while (iter)
        {
            iter = _store->erase(iter);
        }
        break;
    }
    case XenObject::STATUS_UPDATED:
        row[_store->record().colVal] = object->getDisplayStatus();
        break;
    default:
        break;
    }
}


bool HostTreeView::addVm(RefPtr<XenObject> object)
{
    RefPtr<VirtualMachine> vm = RefPtr<VirtualMachine>::castStatic(object);
    XenPtr<xen_vm_record> vmRecord = vm->getRecord();
    if (vmRecord->is_a_template ||
        vmRecord->is_control_domain ||
        vmRecord->is_a_snapshot)
    {
        return false;
    }
    return addObject(object);
}


bool HostTreeView::addObject(RefPtr<XenObject> object)
{
    Gtk::TreeIter iter = find(*object, true);
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        row[_store->record().colPix] = PixStore::instance().get(*object);
        row[_store->record().colKey] = object->getName();
        row[_store->record().colVal] = object->getDisplayStatus();
        row[_store->record().colXenObject] = object;
        return true;
    }
    else
    {
        return false;
    }
}


void HostTreeView::updateObject(RefPtr<XenObject> object, int what)
{
    Gtk::TreeIter iter = find(*object, false);
    if (!iter)
    {
        return;
    }
    Gtk::TreeModel::Row row = *iter;
    switch (what)
    {
    case XenObject::BUSY_SET:
    case XenObject::BUSY_RESET:
    case XenObject::POWER_STATE_UPDATED:
        row[_store->record().colPix] = PixStore::instance().get(*object);
        break;
    case XenObject::STATUS_UPDATED:
        row[_store->record().colVal] = object->getDisplayStatus();
        break;
    case XenObject::NAME_UPDATED:
        row[_store->record().colKey] = object->getName();
        switch (object->getType())
        {
        case XenObject::VM:
        case XenObject::SR:
        case XenObject::NETWORK:
            reorder(*object, iter);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}


bool HostTreeView::addNothing(RefPtr<XenObject> object)
{
    return false;
}


void HostTreeView::updateNothing(RefPtr<XenObject> object, int what)
{
}


Gtk::TreeIter HostTreeView::getFirst() const
{
    return _store->get_iter("0");
}


RefPtr<XenObject> HostTreeView::getObject(const Gtk::TreeIter& iter) const
{
    return (*iter)[_store->record().colXenObject];
}


bool HostTreeView::on_button_press_event(GdkEventButton* event)
{
    bool retval = Gtk::TreeView::on_button_press_event(event);

    if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
    {
        Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
        std::list<Gtk::TreeIter> selected;
        Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
        while (iter)
        {
            if (selection->is_selected(iter))
            {
                selected.push_back(iter);
            }
            Gtk::TreeModel::Row row = *iter;
            Gtk::TreeIter iter2 = row.children().begin();
            while (iter2)
            {
                if (selection->is_selected(iter2))
                {
                    selected.push_back(iter2);
                }
                iter2++;
            }
            iter++;
        }
        if (selected.size() == 1)
        {
            iter = selected.front();
            Gtk::TreeModel::Row row = *iter;
            RefPtr<XenObject> object = row[_store->record().colXenObject];
            switch (object->getType())
            {
            case XenObject::HOST:
                _menuHost.popup(event->button, event->time, RefPtr<Host>::castStatic(object));
                break;
            case XenObject::VM:
                //_menuVm.popup(event->button, event->time, RefPtr<VirtualMachine>::castStatic(object));
                break;
            case XenObject::SR:
                _menuSr.popup(event->button, event->time, static_cast<StorageRepository&>(*object));
                break;
            default:
                break;
            }
        }
        // The event has been handled.
        return true;
    }

    return retval;
}


void HostTreeView::updateDisplayOrder()
{
    int displayOrder = 10;
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        RefPtr<XenObject> object = row[_store->record().colXenObject];
        object->getSession().getConnectSpec().displayOrder = displayOrder;
        displayOrder += 10;
        iter++;
    }
}


void HostTreeView::remove(const XenObject& object)
{
    if (object.getType() == XenObject::HOST)
    {
        remove(object, _store->get_iter("0"));
    }
    else
    {
        Gtk::TreeIter iter = findHost(object, false);
        if (iter)
        {
            remove(object, iter->children().begin());
        }
    }
}


void HostTreeView::remove(const XenObject& object, Gtk::TreeIter iter)
{
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        const XenObject& object2 = *((RefPtr<XenObject>)row[_store->record().colXenObject]);
        if (&object == &object2)
        {
            _store->erase(iter);
            return;
        }
        iter++;
    }
}


void HostTreeView::reorder(const XenObject& object, Gtk::TreeIter iterSource)
{
    Gtk::TreeIter iter1 = findHost(object, false);
    if (!iter1)
    {
        return;
    }
    Gtk::TreeIter iter2 = iter1->children().begin();
    while (iter2)
    {
        if (iter2 != iterSource)
        {
            int d = compare(object, iter2);
            if (d < 0)
            {
                _store->move(iterSource, iter2);
                return;
            }
        }
        iter2++;
    }
    iter2 = _store->append(iter1->children());
    _store->move(iterSource, iter2);
    _store->erase(iter2);
}


Gtk::TreeIter HostTreeView::find(const XenObject& object, bool addIfNotFound)
{
    const XenObject& object0 = *object.getSession().getStore().getHost();
    if (&object == &object0)
    {
        return findHost(object0, addIfNotFound);
    }
    Gtk::TreeIter iter = findHost(object0, false);
    if (iter)
    {
        return findChild(object, iter, addIfNotFound);
    }
    else
    {
        return Gtk::TreeIter();
    }
}


Gtk::TreeIter HostTreeView::findHost(const XenObject& object, bool addIfNotFound)
{
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        int d = compare(object, iter);
        if (d == 0)
        {
            return iter;
        }
        else if (d < 0)
        {
            if (addIfNotFound)
            {
                return _store->insert(iter);
            }
            else
            {
                return Gtk::TreeIter();
            }
        }
        iter++;
    }
    if (addIfNotFound)
    {
        return _store->append();
    }
    else
    {
        return Gtk::TreeIter();
    }
}


Gtk::TreeIter HostTreeView::findChild(const XenObject& object, Gtk::TreeIter iter1, bool addIfNotFound)
{
    Gtk::TreeIter iter2 = iter1->children().begin();
    while (iter2)
    {
        int d = compare(object, iter2);
        if (d == 0)
        {
            return iter2;
        }
        else if (d < 0)
        {
            if (addIfNotFound)
            {
                return _store->insert(iter2);
            }
            else
            {
                return Gtk::TreeIter();
            }
        }
        iter2++;
    }
    if (addIfNotFound)
    {
        iter2 = _store->append(iter1->children());
        expand_row(_store->get_path(iter1), true);
        return iter2;
    }
    else
    {
        return Gtk::TreeIter();
    }
}


static int GetTypeOrder(XenObject::Type type)
{
    return
        type == XenObject::HOST ? 0 :
        type == XenObject::VM ? 1 :
        type == XenObject::SR ? 2 :
        type == XenObject::NETWORK ? 3 :
        4;
}


static int GetSubTypeOrder(StorageRepository::SubType subType)
{
    return 
        subType == StorageRepository::USR ? 0 :
        subType == StorageRepository::DEV ? 1 :
        subType == StorageRepository::ISO ? 2 :
        3;
}


int HostTreeView::compare(const XenObject& object1, const Gtk::TreeIter& iter)
{
    Gtk::TreeModel::Row row = *iter;
    const XenObject& object2 = *((RefPtr<XenObject>)row[_store->record().colXenObject]);
    if (&object1 == &object2)
    {
        return 0;
    }
    int t1 = GetTypeOrder(object1.getType());
    int t2 = GetTypeOrder(object2.getType());
    int dx = t1 - t2;
    if (dx != 0)
    {
        return dx;
    }
    switch (object1.getType())
    {
    case XenObject::HOST:
        t1 = object1.getSession().getConnectSpec().displayOrder;
        t2 = object2.getSession().getConnectSpec().displayOrder;
        dx = t1 - t2;
        if (dx != 0)
        {
            return dx;
        }
        break;
    case XenObject::VM:
        break;
    case XenObject::SR:
        t1 = GetSubTypeOrder(static_cast<const StorageRepository&>(object1).getSubType());
        t2 = GetSubTypeOrder(static_cast<const StorageRepository&>(object2).getSubType());
        dx = t1 - t2;
        if (dx != 0)
        {
            return dx;
        }
        break;
    case XenObject::NETWORK:
        t1 = static_cast<const Network&>(object1).isHostInternalManagement() ? 1 : 0;
        t2 = static_cast<const Network&>(object2).isHostInternalManagement() ? 1 : 0;
        dx = t1 - t2;
        if (dx != 0)
        {
            return dx;
        }
        break;
    default:
        break;
    }
    dx = object1.getName().compare(object2.getName());
    if (dx != 0)
    {
        return dx;
    }
    return object1.getUUID().compare(object2.getUUID());
}

