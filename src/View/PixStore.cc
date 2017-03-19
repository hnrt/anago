// Copyright (C) 2012-2017 Hideaki Narita


#include <gtkmm.h>
#include "Icon/Hourglass.h"
#include "Icon/No.h"
#include "Icon/Pause.h"
#include "Icon/PowerOff.h"
#include "Icon/PowerOn.h"
#include "Icon/Warning.h"
#include "Icon/Yes.h"
#include "XenServer/Host.h"
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
    : _pixApp(Gdk::Pixbuf::create_from_file("/usr/share/icons/gnome/32x32/apps/preferences-desktop-remote-desktop.png"))
    , _pixError(RenderIcon(Gtk::Stock::DIALOG_ERROR, Gtk::ICON_SIZE_BUTTON))
    , _pixHourglass(Gdk::Pixbuf::create_from_inline(-1, _iconHourglass, false))
    , _pixNo(Gdk::Pixbuf::create_from_inline(-1, _iconNo, false))
    , _pixPause(Gdk::Pixbuf::create_from_inline(-1, _iconPause, false))
    , _pixPowerOff(Gdk::Pixbuf::create_from_inline(-1, _iconPowerOff, false))
    , _pixPowerOn(Gdk::Pixbuf::create_from_inline(-1, _iconPowerOn, false))
    , _pixWarning(Gdk::Pixbuf::create_from_inline(-1, _iconWarning, false))
    , _pixYes(Gdk::Pixbuf::create_from_inline(-1, _iconYes, false))
{
}


Glib::RefPtr<Gdk::Pixbuf> PixStore::get(RefPtr<Host> host)
{
    return
        !host ? _pixError :
        host->isBusy() ? _pixHourglass :
        host->getSession().isConnected() ? _pixYes :
        _pixNo;
}


Glib::RefPtr<Gdk::Pixbuf> PixStore::get(RefPtr<StorageRepository> sr)
{
    return _pixError;
}


Glib::RefPtr<Gdk::Pixbuf> PixStore::get(RefPtr<VirtualMachine> vm)
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
