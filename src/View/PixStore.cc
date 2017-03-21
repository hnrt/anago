// Copyright (C) 2012-2017 Hideaki Narita


#include <gtkmm.h>
#include "Icon/CdRom.h"
#include "Icon/HardDisk.h"
#include "Icon/Hourglass.h"
#include "Icon/NetworkAdapter.h"
#include "Icon/No.h"
#include "Icon/Pause.h"
#include "Icon/PowerOff.h"
#include "Icon/PowerOn.h"
#include "Icon/RemoteDesktop.h"
#include "Icon/RemovableMedia.h"
#include "Icon/Warning.h"
#include "Icon/Yes.h"
#include "XenServer/Host.h"
#include "XenServer/Network.h"
#include "XenServer/Session.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/VirtualMachine.h"
#include "PixStore.h"


using namespace hnrt;


static PixStore* _singleton = NULL;


void PixStore::init()
{
    _singleton = new PixStore();
}


void PixStore::fini()
{
    delete _singleton;
}


PixStore& PixStore::instance()
{
    return *_singleton;
}


static Glib::RefPtr<Gdk::Pixbuf> RenderIcon(const Gtk::StockID& stockId, Gtk::IconSize size)
{
    Gtk::Invisible w;
    return w.render_icon(stockId, size);
}


PixStore::PixStore()
    : _pixApp(Gdk::Pixbuf::create_from_inline(-1, _iconRemoteDesktop, false))
    , _pixCdRom(Gdk::Pixbuf::create_from_inline(-1, _iconCdRom, false))
    , _pixError(RenderIcon(Gtk::Stock::DIALOG_ERROR, Gtk::ICON_SIZE_BUTTON))
    , _pixHardDisk(Gdk::Pixbuf::create_from_inline(-1, _iconHardDisk, false))
    , _pixHourglass(Gdk::Pixbuf::create_from_inline(-1, _iconHourglass, false))
    , _pixNetworkAdapter(Gdk::Pixbuf::create_from_inline(-1, _iconNetworkAdapter, false))
    , _pixNo(Gdk::Pixbuf::create_from_inline(-1, _iconNo, false))
    , _pixPause(Gdk::Pixbuf::create_from_inline(-1, _iconPause, false))
    , _pixPowerOff(Gdk::Pixbuf::create_from_inline(-1, _iconPowerOff, false))
    , _pixPowerOn(Gdk::Pixbuf::create_from_inline(-1, _iconPowerOn, false))
    , _pixRemovableMedia(Gdk::Pixbuf::create_from_inline(-1, _iconRemovableMedia, false))
    , _pixWarning(Gdk::Pixbuf::create_from_inline(-1, _iconWarning, false))
    , _pixYes(Gdk::Pixbuf::create_from_inline(-1, _iconYes, false))
{
}


Glib::RefPtr<Gdk::Pixbuf> PixStore::get(RefPtr<Host> host) const
{
    return
        !host ? _pixError :
        host->isBusy() ? _pixHourglass :
        host->getSession().isConnected() ? _pixYes :
        _pixNo;
}


Glib::RefPtr<Gdk::Pixbuf> PixStore::get(RefPtr<StorageRepository> sr) const
{
    if (!sr)
    {
        return _pixError;
    }
    else if (sr->isBusy())
    {
        return _pixHourglass;
    }
    else
    {
        switch (sr->getSubType())
        {
        case StorageRepository::DEV:
            return _pixRemovableMedia;
        case StorageRepository::ISO:
            return _pixCdRom;
        default:
            return _pixHardDisk;
        }
    }
}


Glib::RefPtr<Gdk::Pixbuf> PixStore::get(RefPtr<VirtualMachine> vm) const
{
    if (!vm)
    {
        return _pixError;
    }
    else if (vm->isBusy())
    {
        return _pixHourglass;
    }
    else
    {
        XenPtr<xen_vm_record> record = vm->getRecord();
        return
            //record->is_a_snapshot ? _pixComputer :
            record->power_state == XEN_VM_POWER_STATE_HALTED ? _pixPowerOff :
            record->power_state == XEN_VM_POWER_STATE_PAUSED ? _pixWarning :
            record->power_state == XEN_VM_POWER_STATE_RUNNING ? _pixPowerOn :
            record->power_state == XEN_VM_POWER_STATE_SUSPENDED ? _pixPause :
            _pixError;
    }
}


Glib::RefPtr<Gdk::Pixbuf> PixStore::get(RefPtr<Network> nw) const
{
    if (!nw)
    {
        return _pixError;
    }
    else if (nw->isBusy())
    {
        return _pixHourglass;
    }
    else
    {
        return _pixNetworkAdapter;
    }
}
