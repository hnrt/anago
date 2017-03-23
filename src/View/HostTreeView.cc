// Copyright (C) 2012-2017 Hideaki Narita


#include <stdio.h>
#include <libintl.h>
#include "Controller/Controller.h"
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
    Controller::instance().signalNotified(XenObject::CREATED).connect(sigc::mem_fun(*this, &HostTreeView::onObjectCreated));
}


HostTreeView::~HostTreeView()
{
}


void HostTreeView::clear()
{
    _store.clear();
}


void HostTreeView::onObjectCreated(RefPtr<RefObj> object0, int what)
{
    Trace trace("HostTreeView::onObjectCreated");
    RefPtr<XenObject> object = RefPtr<XenObject>::castStatic(object0);
    AddObject add = getAdd(object);
    if ((this->*add)(object))
    {
        trace.put("add=true");
        Controller::instance().signalNotified(object0).connect(sigc::mem_fun(*this, &HostTreeView::onObjectUpdated));
        _signalNodeCreated.emit(object);
    }
}


void HostTreeView::onObjectUpdated(RefPtr<RefObj> object0, int what)
{
    RefPtr<XenObject> object = RefPtr<XenObject>::castStatic(object0);
    if (what == XenObject::DESTROYED)
    {
        remove(object);
        return;
    }
    else
    {
        UpdateObject update = getUpdate(object);
        (this->*update)(object, what);
    }
}


void HostTreeView::remove(const RefPtr<XenObject>& object)
{
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        switch (object->getType())
        {
        case XenObject::HOST:
            if (object == row[_store->record().colXenObject])
            {
                _store->erase(iter);
                return;
            }
            break;
        case XenObject::VM:
        case XenObject::SR:
        case XenObject::NETWORK:
        {
            Gtk::TreeIter iter2 = row.children().begin();
            while (iter2)
            {
                row = *iter2;
                if (object == row[_store->record().colXenObject])
                {
                    _store->erase(iter2);
                    return;
                }
                iter2++;
            }
            break;
        }
        default:
            break;
        }
        iter++;
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
        return &HostTreeView::addSr;
    case XenObject::NETWORK:
        return &HostTreeView::addNw;
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
        return &HostTreeView::updateVm;
    case XenObject::SR:
        return &HostTreeView::updateSr;
    case XenObject::NETWORK:
        return &HostTreeView::updateNw;
    default:
        return &HostTreeView::updateNothing;
    }
}


bool HostTreeView::addHost(RefPtr<XenObject> object)
{
    Gtk::TreeIter iter;
    Gtk::TreeModel::Row row = findHost(object, iter, true);
    RefPtr<Host> host = RefPtr<Host>::castStatic(object);
    row[_store->record().colPix] = PixStore::instance().get(host);
    row[_store->record().colKey] = object->getSession().getConnectSpec().displayname;
    row[_store->record().colVal] = object->getDisplayStatus();
    row[_store->record().colXenObject] = object;
    return true;
}


void HostTreeView::updateHost(RefPtr<XenObject> object, int what)
{
    try
    {
        Gtk::TreeIter iter;
        Gtk::TreeModel::Row row = findHost(object, iter);
        RefPtr<Host> host = RefPtr<Host>::castStatic(object);
        switch (what)
        {
        case XenObject::NAME_UPDATED:
            row[_store->record().colKey] = object->getSession().getConnectSpec().displayname;
            break;
        case XenObject::BUSY_SET:
        case XenObject::BUSY_RESET:
            row[_store->record().colPix] = PixStore::instance().get(host);
            break;
        case XenObject::CONNECTED:
            row[_store->record().colPix] = PixStore::instance().get(host);
            break;
        case XenObject::DISCONNECTED:
        {
            row[_store->record().colPix] = PixStore::instance().get(host);
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
    catch (...)
    {
    }
}


bool HostTreeView::addVm(RefPtr<XenObject> object)
{
    RefPtr<VirtualMachine> vm = RefPtr<VirtualMachine>::castStatic(object);
    XenPtr<xen_vm_record> vmRecord = vm->getRecord();
    if (!vmRecord ||
        vmRecord->is_a_template ||
        vmRecord->is_control_domain ||
        vmRecord->is_a_snapshot)
    {
        return false;
    }
    Gtk::TreeIter iter;
    Gtk::TreeModel::Row row = findVm(object, iter, true);
    row[_store->record().colPix] = PixStore::instance().get(vm);
    row[_store->record().colKey] = object->getName();
    row[_store->record().colVal] = object->getDisplayStatus();
    row[_store->record().colXenObject] = object;
    return true;
}


void HostTreeView::updateVm(RefPtr<XenObject> object, int what)
{
    try
    {
        Gtk::TreeIter iter;
        Gtk::TreeModel::Row row = findVm(object, iter);
        switch (what)
        {
        case XenObject::BUSY_SET:
        case XenObject::BUSY_RESET:
        case XenObject::POWER_STATE_UPDATED:
            row[_store->record().colPix] = PixStore::instance().get(RefPtr<VirtualMachine>::castStatic(object));
            break;
        case XenObject::STATUS_UPDATED:
            row[_store->record().colVal] = object->getDisplayStatus();
            break;
        case XenObject::NAME_UPDATED:
            row[_store->record().colKey] = object->getName();
            reorder(RefPtr<VirtualMachine>::castStatic(object), iter);
            break;
        default:
            break;
        }
    }
    catch (...)
    {
    }
}


bool HostTreeView::addSr(RefPtr<XenObject> object)
{
    try
    {
        Gtk::TreeIter iter;
        Gtk::TreeModel::Row row = findSr(object, iter, true);
        row[_store->record().colPix] = PixStore::instance().get(RefPtr<StorageRepository>::castStatic(object));
        row[_store->record().colKey] = object->getName();
        row[_store->record().colVal] = object->getDisplayStatus();
        row[_store->record().colXenObject] = object;
        return true;
    }
    catch (...)
    {
        return false;
    }
}


void HostTreeView::updateSr(RefPtr<XenObject> object, int what)
{
    try
    {
        Gtk::TreeIter iter;
        Gtk::TreeModel::Row row = findSr(object, iter);
        switch (what)
        {
        case XenObject::BUSY_SET:
        case XenObject::BUSY_RESET:
            row[_store->record().colPix] = PixStore::instance().get(RefPtr<StorageRepository>::castStatic(object));
            break;
        case XenObject::STATUS_UPDATED:
            row[_store->record().colVal] = object->getDisplayStatus();
            break;
        case XenObject::NAME_UPDATED:
            row[_store->record().colKey] = object->getName();
            reorder(RefPtr<StorageRepository>::castStatic(object), iter);
            break;
        default:
            break;
        }
    }
    catch (...)
    {
    }
}


bool HostTreeView::addNw(RefPtr<XenObject> object)
{
    try
    {
        Gtk::TreeIter iter;
        Gtk::TreeModel::Row row = findNw(object, iter, true);
        row[_store->record().colPix] = PixStore::instance().get(RefPtr<Network>::castStatic(object));
        row[_store->record().colKey] = object->getName();
        row[_store->record().colVal] = object->getDisplayStatus();
        row[_store->record().colXenObject] = object;
        return true;
    }
    catch (...)
    {
        return false;
    }
}


void HostTreeView::updateNw(RefPtr<XenObject> object, int what)
{
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


void HostTreeView::reorder(RefPtr<VirtualMachine> vm, Gtk::TreeIter& iterSource)
{
    RefPtr<Host> host = vm->getSession().getStore().getHost();
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        RefPtr<XenObject> object = row[_store->record().colXenObject];
        if (object->getUUID() == host->getUUID())
        {
            Gtk::TreeIter iter2 = row.children().begin();
            while (iter2)
            {
                row = *iter2;
                object = row[_store->record().colXenObject];
                if (object == RefPtr<XenObject>::castStatic(vm))
                {
                    // nothing to do
                }
                else if (object->getType() == XenObject::VM)
                {
                    Glib::ustring key = row[_store->record().colKey];
                    if (key > vm->getName())
                    {
                        _store->move(iterSource, iter2);
                        return;
                    }
                }
                else if (object->getType() == XenObject::SR)
                {
                    _store->move(iterSource, iter2);
                    return;
                }
                iter2++;
            }
            iter2 = _store->append(iter->children());
            _store->move(iterSource, iter2);
            _store->erase(iter2);
            return;
        }
        iter++;
    }
}


void HostTreeView::reorder(RefPtr<StorageRepository> sr, Gtk::TreeIter& iterSource)
{
    RefPtr<Host> host = sr->getSession().getStore().getHost();
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        RefPtr<XenObject> object = row[_store->record().colXenObject];
        if (object->getUUID() == host->getUUID())
        {
            Gtk::TreeIter iter2 = row.children().begin();
            while (iter2)
            {
                row = *iter2;
                object = row[_store->record().colXenObject];
                if (object.ptr() == sr.ptr())
                {
                    // nothing to do
                }
                else if (object->getType() == XenObject::SR)
                {
                    Glib::ustring key = row[_store->record().colKey];
                    if (sr->getSubType() == StorageRepository::ISO)
                    {
                        if (RefPtr<StorageRepository>::castStatic(object)->getSubType() == StorageRepository::ISO)
                        {
                            if (key > sr->getName())
                            {
                                _store->move(iterSource, iter2);
                                return;
                            }
                        }
                    }
                    else if (sr->getSubType() == StorageRepository::DEV)
                    {
                        if (RefPtr<StorageRepository>::castStatic(object)->getSubType() == StorageRepository::ISO)
                        {
                            _store->move(iterSource, iter2);
                            return;
                        }
                        else if (RefPtr<StorageRepository>::castStatic(object)->getSubType() == StorageRepository::DEV)
                        {
                            if (key > sr->getName())
                            {
                                _store->move(iterSource, iter2);
                                return;
                            }
                        }
                    }
                    else if (RefPtr<StorageRepository>::castStatic(object)->getSubType() == StorageRepository::ISO ||
                             RefPtr<StorageRepository>::castStatic(object)->getSubType() == StorageRepository::DEV ||
                             key > sr->getName())
                    {
                        _store->move(iterSource, iter2);
                        return;
                    }
                }
                iter2++;
            }
            iter2 = _store->append(iter->children());
            _store->move(iterSource, iter2);
            _store->erase(iter2);
            return;
        }
        iter++;
    }
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
                //_menuSr.popup(event->button, event->time, RefPtr<StorageRepository>::castStatic(object));
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


Gtk::TreeModel::Row HostTreeView::findHost(const RefPtr<XenObject>& object, Gtk::TreeIter& iter, bool addIfNotFound)
{
    RefPtr<Host> host = RefPtr<Host>::castStatic(object);
    iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        if (object == row[_store->record().colXenObject])
        {
            return row;
        }
        iter++;
    }
    if (addIfNotFound)
    {
        iter = _store->get_iter("0"); // point to first item
        while (iter)
        {
            Gtk::TreeModel::Row row = *iter;
            RefPtr<XenObject> object2 = row[_store->record().colXenObject];
            if (object->getSession().getConnectSpec().displayOrder < object2->getSession().getConnectSpec().displayOrder)
            {
                return *_store->insert(iter);
            }
            iter++;
        }
        return *_store->append();
    }
    else
    {
        throw "Host not found.";
    }
}


Gtk::TreeModel::Row HostTreeView::findVm(const RefPtr<XenObject>& object, Gtk::TreeIter& iter, bool addIfNotFound)
{
    RefPtr<Host> host = object->getSession().getStore().getHost();
    Gtk::TreeIter iter0;
    Gtk::TreeModel::Row row = findHost(RefPtr<XenObject>::castStatic(host), iter0);
    RefPtr<VirtualMachine> vm = RefPtr<VirtualMachine>::castStatic(object);
    iter = row.children().begin();
    while (iter)
    {
        row = *iter;
        RefPtr<XenObject> object2 = row[_store->record().colXenObject];
        if (object2 == object)
        {
            return row;
        }
        else if (object2->getType() == XenObject::VM)
        {
            Glib::ustring key = row[_store->record().colKey];
            if (key > vm->getName())
            {
                if (addIfNotFound)
                {
                    return *_store->insert(iter);
                }
                else
                {
                    throw "VM not found.";
                }
            }
        }
        else if (addIfNotFound)
        {
            return *_store->insert(iter);
        }
        else
        {
            throw "VM not found.";
        }
        iter++;
    }
    if (addIfNotFound)
    {
        row = *_store->append(iter0->children());
        expand_row(_store->get_path(iter0), true);
        return row;
    }
    else
    {
        throw "VM not found.";
    }
}


Gtk::TreeModel::Row HostTreeView::findSr(const RefPtr<XenObject>& object, Gtk::TreeIter& iter, bool addIfNotFound)
{
    RefPtr<Host> host = object->getSession().getStore().getHost();
    Gtk::TreeIter iter0;
    Gtk::TreeModel::Row row = findHost(RefPtr<XenObject>::castStatic(host), iter0);
    RefPtr<StorageRepository> sr = RefPtr<StorageRepository>::castStatic(object);
    iter = row.children().begin();
    while (iter)
    {
        row = *iter;
        RefPtr<XenObject> object2 = row[_store->record().colXenObject];
        if (object2 == object)
        {
            return row;
        }
        else if (object2->getType() == XenObject::VM)
        {
        }
        else if (object2->getType() == XenObject::SR)
        {
            RefPtr<StorageRepository> sr2 = RefPtr<StorageRepository>::castStatic(object2);
            Glib::ustring key = row[_store->record().colKey];
            if (sr->getSubType() == StorageRepository::ISO)
            {
                if (sr2->getSubType() == StorageRepository::ISO)
                {
                    if (key > sr->getName())
                    {
                        if (addIfNotFound)
                        {
                            return *_store->insert(iter);
                        }
                        else
                        {
                            throw "SR not found.";
                        }
                    }
                }
            }
            else if (sr->getSubType() == StorageRepository::DEV)
            {
                if (sr2->getSubType() == StorageRepository::ISO)
                {
                    if (addIfNotFound)
                    {
                        return *_store->insert(iter);
                    }
                    else
                    {
                        throw "SR not found.";
                    }
                }
                else if (sr2->getSubType() == StorageRepository::DEV)
                {
                    if (key > sr->getName())
                    {
                        if (addIfNotFound)
                        {
                            return *_store->insert(iter);
                        }
                        else
                        {
                            throw "SR not found.";
                        }
                    }
                }
            }
            else if (sr2->getSubType() == StorageRepository::ISO ||
                     sr2->getSubType() == StorageRepository::DEV ||
                     key > sr->getName())
            {
                if (addIfNotFound)
                {
                    return *_store->insert(iter);
                }
                else
                {
                    throw "SR not found.";
                }
            }
        }
        else if (addIfNotFound)
        {
            return *_store->insert(iter);
        }
        else
        {
            throw "SR not found.";
        }
        iter++;
    }
    if (addIfNotFound)
    {
        row = *_store->append(iter0->children());
        expand_row(_store->get_path(iter0), true);
        return row;
    }
    else
    {
        throw "SR not found.";
    }
}


Gtk::TreeModel::Row HostTreeView::findNw(const RefPtr<XenObject>& object, Gtk::TreeIter& iter, bool addIfNotFound)
{
    RefPtr<Host> host = object->getSession().getStore().getHost();
    Gtk::TreeIter iter0;
    Gtk::TreeModel::Row row = findHost(RefPtr<XenObject>::castStatic(host), iter0);
    RefPtr<Network> nw = RefPtr<Network>::castStatic(object);
    XenPtr<xen_network_record> nwRecord = nw->getRecord();
    bool isInternal = nw->isHostInternalManagement();
    iter = row.children().begin();
    while (iter)
    {
        row = *iter;
        RefPtr<XenObject> object2 = row[_store->record().colXenObject];
        if (object2 == object)
        {
            return row;
        }
        else if (object2->getType() == XenObject::VM)
        {
        }
        else if (object2->getType() == XenObject::SR)
        {
        }
        else if (object2->getType() == XenObject::NETWORK)
        {
            Glib::ustring key = row[_store->record().colKey];
            if (!isInternal && key > nwRecord->name_label)
            {
                if (addIfNotFound)
                {
                    return *_store->insert(iter);
                }
                else
                {
                    throw "Network not found.";
                }
            }
        }
        else if (addIfNotFound)
        {
            return *_store->insert(iter);
        }
        else
        {
            throw "Network not found.";
        }
        iter++;
    }
    if (addIfNotFound)
    {
        row = *_store->append(iter0->children());
        expand_row(_store->get_path(iter0), true);
        return row;
    }
    else
    {
        throw "Network not found.";
    }
}
