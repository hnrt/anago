// Copyright (C) 2012-2017 Hideaki Narita


#include <stdio.h>
#include <libintl.h>
#include "XenServer/Host.h"
//#include "XenServer/Network.h"
#include "XenServer/Session.h"
//#include "XenServer/StorageRepository.h"
//#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObjectStore.h"
#include "PixStore.h"
#include "ServerTreeStore.h"
#include "ServerTreeView.h"


using namespace hnrt;


ServerTreeView::ServerTreeView()
{
    _store = ServerTreeStore::create();
    set_model(_store);
    set_headers_visible(false);
    set_reorderable();
    append_column(gettext("Pix"), _store->record().colPix);
    append_column(gettext("ID"), _store->record().colKey);
    append_column(gettext("Contents"), _store->record().colVal);
    Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
    selection->set_mode(Gtk::SELECTION_MULTIPLE);
}


ServerTreeView::~ServerTreeView()
{
}


void ServerTreeView::clear()
{
    _store.clear();
}


bool ServerTreeView::add(RefPtr<XenObject>& object)
{
    XenObject::Type type = object->getType();
    if (type == XenObject::HOST)
    {
        Gtk::TreeModel::Row row;
        Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
        while (iter)
        {
            row = *iter;
            RefPtr<XenObject> object2 = row[_store->record().colXenObject];
            if (object2 == object)
            {
                row[_store->record().colPix] = PixStore::instance().get(RefPtr<Host>::castStatic(object));
                row[_store->record().colVal] = object->getDisplayStatus();
                return false;
            }
            else if (object2->getSession().getConnectSpec().uuid == object->getSession().getConnectSpec().uuid)
            {
                row[_store->record().colPix] = PixStore::instance().get(RefPtr<Host>::castStatic(object));
                row[_store->record().colVal] = object->getDisplayStatus();
                row[_store->record().colXenObject] = object;
                return true;
            }
            iter++;
        }
        iter = _store->get_iter("0"); // point to first item
        while (1)
        {
            if (!iter)
            {
                row = *_store->append();
                break;
            }
            row = *iter;
            RefPtr<XenObject> object2 = row[_store->record().colXenObject];
            if (object->getSession().getConnectSpec().displayOrder < object2->getSession().getConnectSpec().displayOrder)
            {
                row = *_store->insert(iter);
                break;
            }
            iter++;
        }
        row[_store->record().colPix] = PixStore::instance().get(RefPtr<Host>::castStatic(object));
        row[_store->record().colKey] = object->getSession().getConnectSpec().displayname;
        row[_store->record().colVal] = object->getDisplayStatus();
        row[_store->record().colXenObject] = object;
        return true;
    }
    else
    {
        return false;
    }
}


void ServerTreeView::remove(const RefPtr<XenObject>& object)
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
#if 0
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
#endif
        default:
            break;
        }
        iter++;
    }
}


#if 0
bool ServerTreeView::add(const RefPtr<VirtualMachine> vm)
{
    XenPtr<xen_vm_record> vmRecord = vm->getRecord();
    if (!vmRecord ||
        vmRecord->is_a_template ||
        vmRecord->is_control_domain ||
        vmRecord->is_a_snapshot)
    {
        return false;
    }
    //printf("#%p ServerTreeView::add(%s): Started.\n", Glib::Thread::self(), vm->getName().str());
    RefPtr<Host> host = vm->getSession().getStore().getHost();
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        RefPtr<XenObject> object = row[_store->record().colXenObject];
        if (object->getUUID() == host->getUUID())
        {
            Gtk::TreeIter iter2 = row.children().begin();
            while (1)
            {
                if (!iter2)
                {
                    row = *_store->append((*iter).children());
                    expand_row(_store->get_path(iter), true);
                    break;
                }
                row = *iter2;
                object = row[_store->record().colXenObject];
                if (object == RefPtr<XenObject>::castStatic(vm))
                {
                    return false;
                }
                else if (object->getType() == XENOBJECT_VM)
                {
                    Glib::ustring key = row[_store->record().colKey];
                    if (key > vm->getName())
                    {
                        row = *_store->insert(iter2);
                        break;
                    }
                }
                else
                {
                    row = *_store->insert(iter2);
                    break;
                }
                iter2++;
            }
            row[_store->record().colPix] = PixStore::instance().get(vm);
            row[_store->record().colKey] = Glib::ustring(vm->getName().str());
            row[_store->record().colVal] = Glib::ustring(vm->getDisplayStatus().str());
            row[_store->record().colXenObject] = RefPtr<XenObject>::castStatic(vm);
            //printf("#%p ServerTreeView::add(%s): Finished.\n", Glib::Thread::self(), vm->getName().str());
            return true;
        }
        iter++;
    }
    // cannot find appropriate insertion point
    return false;
}


bool ServerTreeView::add(const RefPtr<StorageRepository> sr)
{
    //printf("#%p ServerTreeView::add(%s): Started.\n", Glib::Thread::self(), sr->getName().str());
    RefPtr<Host> host = sr->getSession().getStore().getHost();
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        RefPtr<XenObject> object = row[_store->record().colXenObject];
        if (object->getUUID() == host->getUUID())
        {
            Gtk::TreeIter iter2 = row.children().begin();
            while (1)
            {
                if (!iter2)
                {
                    row = *_store->append((*iter).children());
                    expand_row(_store->get_path(iter), true);
                    break;
                }
                row = *iter2;
                object = row[_store->record().colXenObject];
                if (object == RefPtr<XenObject>::castStatic(sr))
                {
                    return false;
                }
                else if (object->getType() == XENOBJECT_VM)
                {
                }
                else if (object->getType() == XENOBJECT_SR)
                {
                    Glib::ustring key = row[_store->record().colKey];
                    if (sr->getSubType() == SR_ISO)
                    {
                        if (RefPtr<StorageRepository>::castStatic(object)->getSubType() == SR_ISO)
                        {
                            if (key > sr->getName())
                            {
                                row = *_store->insert(iter2);
                                break;
                            }
                        }
                    }
                    else if (sr->getSubType() == SR_DEV)
                    {
                        if (RefPtr<StorageRepository>::castStatic(object)->getSubType() == SR_ISO)
                        {
                            row = *_store->insert(iter2);
                            break;
                        }
                        else if (RefPtr<StorageRepository>::castStatic(object)->getSubType() == SR_DEV)
                        {
                            if (key > sr->getName())
                            {
                                row = *_store->insert(iter2);
                                break;
                            }
                        }
                    }
                    else if (RefPtr<StorageRepository>::castStatic(object)->getSubType() == SR_ISO ||
                             RefPtr<StorageRepository>::castStatic(object)->getSubType() == SR_DEV ||
                             key > sr->getName())
                    {
                        row = *_store->insert(iter2);
                        break;
                    }
                }
                else
                {
                    row = *_store->insert(iter2);
                    break;
                }
                iter2++;
            }
            row[_store->record().colPix] = PixStore::instance().get(sr);
            row[_store->record().colKey] = Glib::ustring(sr->getName().str());
            row[_store->record().colVal] = Glib::ustring(sr->getDisplayStatus().str());
            row[_store->record().colXenObject] = RefPtr<XenObject>::castStatic(sr);
            //printf("#%p ServerTreeView::add(%s): Finished.\n", Glib::Thread::self(), sr->getName().str());
            return true;
        }
        iter++;
    }
    // cannot find appropriate insertion point
    return false;
}


bool ServerTreeView::add(const RefPtr<Network> nw)
{
    //printf("#%p ServerTreeView::add(%s): Started.\n", Glib::Thread::self(), nw->getName().str());
    RefPtr<Host> host = nw->getSession().getStore().getHost();
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        RefPtr<XenObject> object = row[_store->record().colXenObject];
        if (object->getUUID() == host->getUUID())
        {
            XenPtr<xen_network_record> nwRecord = nw->getRecord();
            bool isInternal = nw->isHostInternalManagement();
            Gtk::TreeIter iter2 = row.children().begin();
            while (1)
            {
                if (!iter2)
                {
                    row = *_store->append((*iter).children());
                    expand_row(_store->get_path(iter), true);
                    break;
                }
                row = *iter2;
                object = row[_store->record().colXenObject];
                if (object == RefPtr<XenObject>::castStatic(nw))
                {
                    return false;
                }
                else if (object->getType() == XENOBJECT_VM)
                {
                }
                else if (object->getType() == XENOBJECT_SR)
                {
                }
                else if (object->getType() == XENOBJECT_NETWORK)
                {
                    Glib::ustring key = row[_store->record().colKey];
                    if (!isInternal && key > nwRecord->name_label)
                    {
                        row = *_store->insert(iter2);
                        break;
                    }
                }
                else
                {
                    row = *_store->insert(iter2);
                    break;
                }
                iter2++;
            }
            row[_store->record().colPix] = PixStore::instance().get(nw);
            row[_store->record().colKey] = Glib::ustring(nwRecord->name_label);
            row[_store->record().colVal] = Glib::ustring(nw->getDisplayStatus().str());
            row[_store->record().colXenObject] = RefPtr<XenObject>::castStatic(nw);
            //printf("#%p ServerTreeView::add(%s): Finished.\n", Glib::Thread::self(), nw->getName().str());
            return true;
        }
        iter++;
    }
    // cannot find appropriate insertion point
    return false;
}
#endif

void ServerTreeView::update(RefPtr<XenObject>& object, int what)
{
    switch (object->getType())
    {
    case XenObject::HOST:
    {
        Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
        while (iter)
        {
            Gtk::TreeModel::Row row = *iter;
            if (object == row[_store->record().colXenObject])
            {
                switch (what)
                {
                case XenObject::NAME_UPDATED:
                    row[_store->record().colKey] = object->getSession().getConnectSpec().displayname;
                    break;
                case XenObject::BUSY_UPDATED:
                    row[_store->record().colPix] = PixStore::instance().get(RefPtr<Host>::castStatic(object));
                    break;
                case XenObject::CONNECTED:
                    row[_store->record().colPix] = PixStore::instance().get(RefPtr<Host>::castStatic(object));
                    break;
                case XenObject::DISCONNECTED:
                {
                    row[_store->record().colPix] = PixStore::instance().get(RefPtr<Host>::castStatic(object));
                    Gtk::TreeIter iter2 = row.children().begin();
                    while (iter2)
                    {
                        iter2 = _store->erase(iter2);
                    }
                    break;
                }
                case XenObject::STATUS_UPDATED:
                    row[_store->record().colVal] = object->getDisplayStatus();
                    break;
                default:
                    break;
                }
                break;
            }
            iter++;
        }
        break;
    }
#if 0
    case XenObject::VM:
    {
        //printf("#ServerTreeView::update: VM finished.\n");
        RefPtr<VirtualMachine> vm = RefPtr<VirtualMachine>::castStatic(object);
        RefPtr<Host> host = vm->getSession().getStore().getHost();
        Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
        while (iter)
        {
            Gtk::TreeModel::Row row = *iter;
            RefPtr<XenObject> current = row[_store->record().colXenObject];
            if (current->getUUID() == host->getUUID())
            {
                Gtk::TreeIter iter2 = row.children().begin();
                while (iter2)
                {
                    row = *iter2;
                    if (object == row[_store->record().colXenObject])
                    {
                        switch (what)
                        {
                        case NOTIF_XO_BUSY:
                        case NOTIF_XO_POWER_STATE:
                            row[_store->record().colPix] = PixStore::instance().get(vm);
                            break;
                        case NOTIF_XO_STATUS:
                            row[_store->record().colVal] = Glib::ustring(vm->getDisplayStatus().str());
                            break;
                        case NOTIF_XO_NAME:
                            row[_store->record().colKey] = Glib::ustring(vm->getName().str());
                            reorder(vm, iter2);
                            break;
                        default:
                            break;
                        }
                        break;
                    }
                    iter2++;
                }
                break;
            }
            iter++;
        }
        //printf("#ServerTreeView::update: VM finished.\n");
        break;
    }
    case XenObject::SR:
    {
        //printf("#ServerTreeView::update: SR Started.\n");
        RefPtr<StorageRepository> sr = RefPtr<StorageRepository>::castStatic(object);
        RefPtr<Host> host = sr->getSession().getStore().getHost();
        Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
        while (iter)
        {
            Gtk::TreeModel::Row row = *iter;
            RefPtr<XenObject> current = row[_store->record().colXenObject];
            if (current->getUUID() == host->getUUID())
            {
                Gtk::TreeIter iter2 = row.children().begin();
                while (iter2)
                {
                    row = *iter2;
                    if (object == row[_store->record().colXenObject])
                    {
                        switch (what)
                        {
                        case NOTIF_XO_BUSY:
                            row[_store->record().colPix] = PixStore::instance().get(sr);
                            break;
                        case NOTIF_XO_STATUS:
                            row[_store->record().colVal] = Glib::ustring(sr->getDisplayStatus().str());
                            break;
                        case NOTIF_XO_NAME:
                            row[_store->record().colKey] = Glib::ustring(sr->getName().str());
                            reorder(sr, iter2);
                            break;
                        default:
                            break;
                        }
                        break;
                    }
                    iter2++;
                }
                break;
            }
            iter++;
        }
        //printf("#ServerTreeView::update: SR finished.\n");
        break;
    }
#endif
    default:
        break;
    }
}


Gtk::TreeIter ServerTreeView::getFirst() const
{
    return _store->get_iter("0");
}

#if 0
void ServerTreeView::reorder(const RefPtr<VirtualMachine> vm, Gtk::TreeIter& iterSource)
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
                else if (object->getType() == XENOBJECT_VM)
                {
                    Glib::ustring key = row[_store->record().colKey];
                    if (key > vm->getName())
                    {
                        _store->move(iterSource, iter2);
                        return;
                    }
                }
                else if (object->getType() == XENOBJECT_SR)
                {
                    _store->move(iterSource, iter2);
                    return;
                }
                iter2++;
            }
            iter2 = _store->append((*iter).children());
            _store->move(iterSource, iter2);
            _store->erase(iter2);
            return;
        }
        iter++;
    }
    // cannot find appropriate insertion point
}


void ServerTreeView::reorder(const RefPtr<StorageRepository> sr, Gtk::TreeIter& iterSource)
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
                if (object.ptr == sr.ptr)
                {
                    // nothing to do
                }
                else if (object->getType() == XENOBJECT_SR)
                {
                    Glib::ustring key = row[_store->record().colKey];
                    if (sr->getSubType() == SR_ISO)
                    {
                        if (RefPtr<StorageRepository>::castStatic(object)->getSubType() == SR_ISO)
                        {
                            if (key > sr->getName())
                            {
                                _store->move(iterSource, iter2);
                                return;
                            }
                        }
                    }
                    else if (sr->getSubType() == SR_DEV)
                    {
                        if (RefPtr<StorageRepository>::castStatic(object)->getSubType() == SR_ISO)
                        {
                            _store->move(iterSource, iter2);
                            return;
                        }
                        else if (RefPtr<StorageRepository>::castStatic(object)->getSubType() == SR_DEV)
                        {
                            if (key > sr->getName())
                            {
                                _store->move(iterSource, iter2);
                                return;
                            }
                        }
                    }
                    else if (RefPtr<StorageRepository>::castStatic(object)->getSubType() == SR_ISO ||
                             RefPtr<StorageRepository>::castStatic(object)->getSubType() == SR_DEV ||
                             key > sr->getName())
                    {
                        _store->move(iterSource, iter2);
                        return;
                    }
                }
                iter2++;
            }
            iter2 = _store->append((*iter).children());
            _store->move(iterSource, iter2);
            _store->erase(iter2);
            return;
        }
        iter++;
    }
    // cannot find appropriate insertion point
}
#endif

RefPtr<XenObject> ServerTreeView::getObject(const Gtk::TreeIter& iter) const
{
    return (*iter)[_store->record().colXenObject];
}


bool ServerTreeView::on_button_press_event(GdkEventButton* event)
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
                //_menuServer.popup(event->button, event->time, RefPtr<Host>::castStatic(object));
                break;
#if 0
            case XenObject::VM:
                _menuVm.popup(event->button, event->time, RefPtr<VirtualMachine>::castStatic(object));
                break;
            case XenObject::SR:
                _menuSr.popup(event->button, event->time, RefPtr<StorageRepository>::castStatic(object));
                break;
#endif
            default:
                break;
            }
        }
        // The event has been handled.
        return true;
    }

    return retval;
}


void ServerTreeView::updateDisplayOrder()
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
