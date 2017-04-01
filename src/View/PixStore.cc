// Copyright (C) 2012-2017 Hideaki Narita


#include <gtkmm.h>
#include "Icon/CdRom.h"
#include "Icon/Computer.h"
#include "Icon/HardDisk.h"
#include "Icon/Hourglass.h"
#include "Icon/Memory.h"
#include "Icon/NetworkAdapter.h"
#include "Icon/No.h"
#include "Icon/Pause.h"
#include "Icon/PowerOff.h"
#include "Icon/PowerOn.h"
#include "Icon/RemoteDesktop.h"
#include "Icon/RemovableMedia.h"
#include "Icon/Warning.h"
#include "Icon/Yes.h"
#include "XenServer/Api.h"
#include "XenServer/Host.h"
#include "XenServer/Network.h"
#include "XenServer/Session.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/VirtualBlockDevice.h"
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
    , _pixComputer(Gdk::Pixbuf::create_from_inline(-1, _iconComputer, false))
    , _pixError(RenderIcon(Gtk::Stock::DIALOG_ERROR, Gtk::ICON_SIZE_BUTTON))
    , _pixHardDisk(Gdk::Pixbuf::create_from_inline(-1, _iconHardDisk, false))
    , _pixHourglass(Gdk::Pixbuf::create_from_inline(-1, _iconHourglass, false))
    , _pixMemory(Gdk::Pixbuf::create_from_inline(-1, _iconMemory, false))
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


Glib::RefPtr<Gdk::Pixbuf> PixStore::get(const XenObject& object) const
{
    if (object.isBusy())
    {
        return _pixHourglass;
    }
    switch (object.getType())
    {
    case XenObject::HOST:
        if (object.getSession().isConnected())
        {
            return _pixYes;
        }
        else
        {
            return _pixNo;
        }
    case XenObject::VM:
    {
        XenPtr<xen_vm_record> record = static_cast<const VirtualMachine&>(object).getRecord();
        return
            //record->is_a_snapshot ? _pixComputer :
            record->power_state == XEN_VM_POWER_STATE_HALTED ? _pixPowerOff :
            record->power_state == XEN_VM_POWER_STATE_PAUSED ? _pixWarning :
            record->power_state == XEN_VM_POWER_STATE_RUNNING ? _pixPowerOn :
            record->power_state == XEN_VM_POWER_STATE_SUSPENDED ? _pixPause :
            _pixError;
    }
    case XenObject::SR:
        switch (static_cast<const StorageRepository&>(object).getSubType())
        {
        case StorageRepository::USR:
            return _pixHardDisk;
        case StorageRepository::DEV:
            return _pixRemovableMedia;
        case StorageRepository::ISO:
            return _pixCdRom;
        default:
            return _pixError;
        }
    case XenObject::VBD:
        switch (static_cast<const VirtualBlockDevice&>(object).getRecord()->type)
        {
        case XEN_VBD_TYPE_CD: return _pixRemovableMedia;
        case XEN_VBD_TYPE_DISK: return _pixHardDisk;
        default: return _pixError;
        }
    case XenObject::NETWORK:
    case XenObject::VIF:
        return _pixNetworkAdapter;
    default:
        return _pixError;
    }
}
