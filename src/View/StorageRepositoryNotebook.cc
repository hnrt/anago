// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Base/StringBuffer.h"
#include "Controller/SignalManager.h"
#include "Logger/Trace.h"
#include "XenServer/PhysicalBlockDevice.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/XenPtr.h"
#include "PropertyHelper.h"
#include "StorageRepositoryNotebook.h"


using namespace hnrt;


RefPtr<Notebook> StorageRepositoryNotebook::create(const RefPtr<StorageRepository>& sr)
{
    return RefPtr<Notebook>(new StorageRepositoryNotebook(sr));
}


StorageRepositoryNotebook::StorageRepositoryNotebook(const RefPtr<StorageRepository>& sr)
    : _sr(sr)
{
    Trace trace(StringBuffer().format("StorageRepositoryNotebook@%zx::ctor", this));

    _srLabel.set_label(gettext("Storage repository:"));
    _srLabel.set_alignment(0, 0.5); // h=left, v=center
    _srBox.pack_start(_srLabel, Gtk::PACK_SHRINK);
    _srBox.pack_start(*Gtk::manage(_srLv.createScrolledWindow()));

    _pbdLabel.set_label(gettext("Physical block device:"));
    _pbdLabel.set_alignment(0, 0.5); // h=left, v=center
    _pbdBox.pack_start(_pbdLabel, Gtk::PACK_SHRINK);
    _pbdBox.pack_start(*Gtk::manage(_pbdLv.createScrolledWindow()));

    _genBox.pack1(_srBox, true, true);
    _genBox.pack2(_pbdBox, true, true);
    append_page(_genBox, Glib::ustring(gettext("Properties")));

    _vdiBox.pack_start(*Gtk::manage(_vdiLv.createScrolledWindow()));
    append_page(_vdiBox, Glib::ustring(gettext("Virtual disk images")));

    show_all_children();

    _connection = SignalManager::instance().xenObjectSignal(*_sr).connect(sigc::mem_fun(*this, &StorageRepositoryNotebook::onSrUpdated));

    _srLv.setMenu(&_srMenu);
}


StorageRepositoryNotebook::~StorageRepositoryNotebook()
{
    Trace trace(StringBuffer().format("StorageRepositoryNotebook@%zx::dtor", this));

    _connection.disconnect();
}


void StorageRepositoryNotebook::onSrUpdated(RefPtr<XenObject> object, int what)
{
    XenPtr<xen_sr_record> srRecord = _sr->getRecord();
    SetSrProperties(_srLv, srRecord);

    RefPtr<PhysicalBlockDevice> pbd = _sr->getPbd();
    if (pbd)
    {
        XenPtr<xen_pbd_record> pbdRecord = pbd->getRecord();
        SetPbdProperties(_pbdLv, pbdRecord);
    }
    else
    {
        _pbdLv.clear();
    }

    _vdiLv.update(_sr->getSession(), srRecord->vdis);
}
