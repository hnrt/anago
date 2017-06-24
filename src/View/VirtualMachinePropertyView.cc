// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Controller/SignalManager.h"
#include "Logger/Trace.h"
#include "XenServer/Network.h"
#include "XenServer/PhysicalInterface.h"
#include "XenServer/Session.h"
#include "XenServer/VirtualBlockDevice.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/VirtualInterface.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObjectStore.h"
#include "NetworkMenu.h"
#include "PixStore.h"
#include "PropertyHelper.h"
#include "VirtualDiskImageMenu.h"
#include "VirtualMachineDevicePage.h"
#include "VirtualMachinePropertyView.h"


using namespace hnrt;


VirtualMachinePropertyView::VirtualMachinePropertyView(const RefPtr<VirtualMachine>& vm)
    : _left(*this)
    , _selected(NULL)
    , _genSw(*Gtk::manage(_genLv.createScrolledWindow()))
    , _genMenu(vm)
    , _memSw(*Gtk::manage(_memLv.createScrolledWindow()))
    , _memMenu(vm)
    , _vm(vm)
{
    for (int i = 0; i < MAX_STORAGES; i++)
    {
        _storages[i] = NULL;
    }
    for (int i = 0; i < MAX_NETWORKS; i++)
    {
        _networks[i] = NULL;
    }

    pack1(_left, false, true);
    pack2(_right, true, true);

    _left.signalSelectionChanged().connect(sigc::mem_fun(*this, &VirtualMachinePropertyView::onSelectionChanged));

    _connection = SignalManager::instance().xenObjectSignal(*_vm).connect(sigc::mem_fun(*this, &VirtualMachinePropertyView::onUpdated));
}


VirtualMachinePropertyView::~VirtualMachinePropertyView()
{
    _connection.disconnect();

    for (int i = 0; i < MAX_STORAGES; i++)
    {
        delete _storages[i];
    }
    for (int i = 0; i < MAX_NETWORKS; i++)
    {
        delete _networks[i];
    }
}


void VirtualMachinePropertyView::init()
{
    _genSw.show_all_children();
    _genSw.hide();
    _right.pack_start(_genSw);
    _genLv.setMenu(&_genMenu);

    _memSw.show_all_children();
    _memSw.hide();
    _right.pack_start(_memSw);
    _memLv.setMenu(&_memMenu);

    update();

    _left.select(GENERAL);
}


void VirtualMachinePropertyView::onSelectionChanged()
{
    if (_selected)
    {
        _selected->hide();
    }

    int key = _left.getSelected();
    switch (key)
    {
    case GENERAL:
        _selected = &_genSw;
        break;
    case MEMORY:
        _selected = &_memSw;
        break;
    default:
        if (STORAGE <= key && key < STORAGE + MAX_STORAGES)
        {
            _selected = _storages[key - STORAGE];
        }
        else if (NETWORK <= key && key < NETWORK + MAX_NETWORKS)
        {
            _selected = _networks[key - NETWORK];
        }
        else
        {
            _selected = NULL;
        }
        break;
    }

    if (_selected)
    {
        _selected->show();
    }
}


void VirtualMachinePropertyView::onUpdated(RefPtr<XenObject> object, int what)
{
    TRACEFUN(this, "VirtualMachinePropertyView::onUpdated");

    update();
}


void VirtualMachinePropertyView::update()
{
    Session& session = _vm->getSession();

    XenPtr<xen_vm_record> record = _vm->getRecord();
    XenPtr<xen_vm_metrics_record> metricsRecord = _vm->getMetricsRecord();
    XenPtr<xen_vm_guest_metrics_record> guestMetricsRecord = _vm->getGuestMetricsRecord();

    SetVmProperties(_genLv, record, guestMetricsRecord);

    SetVmMemoryProperties(_memLv, record, metricsRecord);

    int storageFlags[MAX_STORAGES] = { 0 };
    int networkFlags[MAX_NETWORKS] = { 0 };

    for (size_t j = 0, n = record->vbds ? record->vbds->size : 0; j < n; j++)
    {
        RefPtr<VirtualBlockDevice> vbd = session.getStore().getVbd(record->vbds->contents[j]);
        if (!vbd)
        {
            continue;
        }
        XenPtr<xen_vbd_record> vbdRecord = vbd->getRecord();
        if (!vbdRecord)
        {
            continue;
        }
        int i = (int)strtoul(vbdRecord->userdevice, NULL, 10);
        if (i < 0 || MAX_STORAGES <= i || storageFlags[i])
        {
            continue;
        }
        VirtualMachineDevicePage* page = _storages[i];
        if (!page)
        {
            page = _storages[i] = new VirtualMachineDevicePage(vbdRecord->uuid, gettext("Virtual block device:"), gettext("Virtual disk image:"));
            page->show_all_children();
            page->hide();
            Glib::RefPtr<Gdk::Pixbuf> pix = PixStore::instance().get(*vbd);
            Glib::ustring name = Glib::ustring::compose("%1 %2", XenServer::getText(vbdRecord->type), i);
            _left.addEntry(pix, STORAGE + i, name);
            _right.pack_end(*page);
        }
        else if (page->uuid() != vbdRecord->uuid)
        {
            _left.removeEntry(STORAGE + i);
            Glib::RefPtr<Gdk::Pixbuf> pix = PixStore::instance().get(*vbd);
            Glib::ustring name = Glib::ustring::compose("%1 %2", XenServer::getText(vbdRecord->type), i);
            _left.addEntry(pix, STORAGE + i, name);
        }
        SetVbdProperties(page->listView(0), vbdRecord);
        page->setMenu(0, new VirtualBlockDeviceMenu(vbd));
        XenPtr<xen_vdi_record> vdiRecord;
        RefPtr<VirtualDiskImage> vdi = session.getStore().getVdi(vbdRecord->vdi);
        if (vdi)
        {
            page->setMenu(1, new VirtualDiskImageMenu(vdi));
            vdiRecord = vdi->getRecord();
        }
        if (vdiRecord)
        {
            SetVdiProperties(page->listView(1), vdiRecord);
        }
        else
        {
            page->listView(1).clear();
        }
        storageFlags[i] = 1;
    }

    for (int i = 0; i < MAX_STORAGES; i++)
    {
        if (!storageFlags[i] && _storages[i])
        {
            _left.removeEntry(STORAGE + i);
            _right.remove(*_storages[i]);
            delete _storages[i];
            _storages[i] = NULL;
        }
    }

    for (size_t j = 0, n = record->vifs ? record->vifs->size : 0; j < n; j++)
    {
        RefPtr<VirtualInterface> vif = session.getStore().getVif(record->vifs->contents[j]);
        if (!vif)
        {
            continue;
        }
        XenPtr<xen_vif_record> vifRecord = vif->getRecord();
        int i = (int)strtoul(vifRecord->device, NULL, 10);
        if (i < 0 || MAX_NETWORKS <= i || networkFlags[i])
        {
            continue;
        }
        VirtualMachineDevicePage* page = _networks[i];
        if (!page)
        {
            page = _networks[i] = new VirtualMachineDevicePage(vifRecord->uuid, gettext("Virtual interface:"), gettext("Network:"), gettext("Physical interface:"));
            page->show_all_children();
            page->hide();
            _left.addEntry(PixStore::instance().get(*vif), NETWORK + i, vif->getDeviceName());
            _right.pack_end(*page);
        }
        SetVifProperties(page->listView(0), vifRecord, guestMetricsRecord);
        XenPtr<xen_pif_record> pifRecord;
        RefPtr<Network> network = session.getStore().getNw(vifRecord->network);
        if (network)
        {
            XenPtr<xen_network_record> nwRecord = network->getRecord();
            if (nwRecord)
            {
                SetNetworkProperties(page->listView(1), nwRecord);
                page->setMenu(1, new NetworkMenu(network));
                if (nwRecord->pifs && nwRecord->pifs->size)
                {
                    RefPtr<PhysicalInterface> pif = session.getStore().getPif(nwRecord->pifs->contents[0]);
                    if (pif)
                    {
                        pifRecord = pif->getRecord();
                    }
                }
            }
        }
        else
        {
            page->listView(1).clear();
        }
        if (pifRecord)
        {
            SetPifProperties(page->listView(2), pifRecord);
        }
        else
        {
            page->listView(2).clear();
        }
        networkFlags[i] = 1;
    }

    for (int i = 0; i < MAX_NETWORKS; i++)
    {
        if (!networkFlags[i] && _networks[i])
        {
            _left.removeEntry(NETWORK + i);
            _right.remove(*_networks[i]);
            delete _networks[i];
            _networks[i] = NULL;
        }
    }
}


VirtualMachinePropertyView::LeftPane::LeftPane(VirtualMachinePropertyView& parent)
    : _tree(parent)
{
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    set_size_request(150, -1);
    add(_tree);
}


VirtualMachinePropertyView::LeftPane::TreeView::TreeView(VirtualMachinePropertyView& parent)
    : _parent(parent)
{
    _store = Gtk::TreeStore::create(_record);
    set_model(_store);
    set_headers_visible(false);
    append_column(gettext("Pix"), _record.colPix);
    append_column(gettext("Name"), _record.colName);
    get_column(0)->set_reorderable();
    Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
    selection->set_mode(Gtk::SELECTION_SINGLE);

    addEntry(PixStore::instance().getComputer(), GENERAL, gettext("Virtual machine"));
    addEntry(PixStore::instance().getMemory(), MEMORY, gettext("Memory"));
}


void VirtualMachinePropertyView::LeftPane::TreeView::addEntry(Glib::RefPtr<Gdk::Pixbuf> pix, int key, const Glib::ustring& name)
{
    Gtk::TreeModel::Row row;
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    if (key < STORAGE)
    {
        while (iter)
        {
            row = *iter;
            int keyNext = row[_record.colKey];
            if (keyNext < STORAGE)
            {
                if (key < keyNext)
                {
                    row = *_store->insert(iter);
                    goto done;
                }
                else if (key == keyNext)
                {
                    return;
                }
                else
                {
                    iter++;
                }
            }
            else
            {
                break;
            }
        }
    }
    else if (STORAGE <= key && key < STORAGE + MAX_STORAGES)
    {
        while (iter)
        {
            row = *iter;
            int keyNext = row[_record.colKey];
            if (keyNext < STORAGE)
            {
                iter++;
            }
            else if (keyNext < STORAGE + MAX_STORAGES)
            {
                if (key < keyNext)
                {
                    row = *_store->insert(iter);
                    goto done;
                }
                else if (key == keyNext)
                {
                    return;
                }
                else
                {
                    iter++;
                }
            }
            else
            {
                row = *_store->insert(iter);
                goto done;
            }
        }
    }
    else if (NETWORK <= key && key < NETWORK + MAX_NETWORKS)
    {
        while (iter)
        {
            row = *iter;
            int keyNext = row[_record.colKey];
            if (keyNext < NETWORK)
            {
                iter++;
            }
            else if (keyNext < NETWORK + MAX_NETWORKS)
            {
                if (key < keyNext)
                {
                    row = *_store->insert(iter);
                    goto done;
                }
                else if (key == keyNext)
                {
                    return;
                }
                else
                {
                    iter++;
                }
            }
            else
            {
                break;
            }
        }
    }
    row = *_store->append();
done:
    row[_record.colPix] = pix;
    row[_record.colKey] = key;
    row[_record.colName] = name;
}


void VirtualMachinePropertyView::LeftPane::TreeView::removeEntry(int key)
{
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        int keyNext = row[_record.colKey];
        if (keyNext == key)
        {
            _store->erase(iter);
            return;
        }
        iter++;
    }
}


void VirtualMachinePropertyView::LeftPane::TreeView::select(int key)
{
    Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        int keyNext = row[_record.colKey];
        if (keyNext == key)
        {
            selection->select(iter);
            return;
        }
        iter++;
    }
    selection->unselect_all();
}


int VirtualMachinePropertyView::LeftPane::TreeView::getSelected()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
    Gtk::TreeIter iter = selection->get_selected();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        int key = row[_record.colKey];
        return key;
    }
    else
    {
        return NO_SELECTION;
    }
}


bool VirtualMachinePropertyView::LeftPane::TreeView::on_button_press_event(GdkEventButton* event)
{
    bool retval = Gtk::TreeView::on_button_press_event(event);

    if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
    {
        Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
        Gtk::TreeIter iter = selection->get_selected();
        if (iter)
        {
            Gtk::TreeModel::Row row = *iter;
            int key = row[_record.colKey];
            switch (key)
            {
            case GENERAL:
                _parent._genMenu.popup(event->button, event->time);
                break;
            case MEMORY:
                _parent._memMenu.popup(event->button, event->time);
                break;
            default:
                if (STORAGE <= key && key < STORAGE + MAX_STORAGES)
                {
                    _parent._vbdMenu.popup(event->button, event->time, _parent._vm->getSession().getStore().getVbd(_parent._storages[key - STORAGE]->uuid()));
                }
                else if (NETWORK <= key && key < NETWORK + MAX_NETWORKS)
                {
                    _parent._vifMenu.popup(event->button, event->time, _parent._vm->getSession().getStore().getVif(_parent._networks[key - NETWORK]->uuid()));
                }
                break;
            }
        }
        // The event has been handled.
        return true;
    }

    return retval;
}


VirtualMachinePropertyView::LeftPane::TreeView::Record::Record()
{
    add(colPix);
    add(colKey);
    add(colName);
}
