// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Controller/Controller.h"
#include "Controller/SignalManager.h"
#include "XenServer/Session.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/VirtualBlockDevice.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObject.h"
#include "XenServer/XenObjectStore.h"
#include "AttachHddDialog.h"


using namespace hnrt;


AttachHddDialog::AttachHddDialog(Gtk::Window& parent, const VirtualMachine& vm)
    : Gtk::Dialog(gettext("Attach HDD to VM"), parent)
    , _vm(vm)
    , _devLabel(gettext("Device:"))
    , _devCombo(_vm)
    , _srLabel(gettext("Storage repository:"))
    , _srCombo(_vm.getSession())
    , _vdiLabel(gettext("Virtual disk image:"))
    , _addButton(gettext("A_dd"))
    , _resizeButton(gettext("Re_size"))
    , _removeButton(gettext("_Remove"))
{
    set_default_size(-1, 400);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    _pApplyButton = add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();

    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _devBox.pack_start(_devCombo, Gtk::PACK_SHRINK);
    _devBox.set_spacing(6);

    _srBox.pack_start(_srCombo, Gtk::PACK_SHRINK);
    _srBox.set_spacing(6);

    _addButton.set_use_underline();
    _resizeButton.set_use_underline();
    _removeButton.set_use_underline();
    _buttonBox.pack_start(_addButton, Gtk::PACK_SHRINK);
    _buttonBox.pack_start(_resizeButton, Gtk::PACK_SHRINK);
    _buttonBox.pack_start(_removeButton, Gtk::PACK_SHRINK);
    _buttonBox.set_spacing(6);

    _table.attach(_devLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_devBox, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_srLabel, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
    _table.attach(_srBox, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_vdiLabel, 0, 1, 2, 3, Gtk::FILL, Gtk::FILL);
    _table.attach(*Gtk::manage(_vdiList.createScrolledWindow()), 1, 2, 2, 3);
    _table.attach(_buttonBox, 2, 3, 2, 3, Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);

    _devLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _srLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _vdiLabel.set_alignment(1.0, 0.0); // h=right, v=top

    _srCombo.signal_changed().connect(sigc::mem_fun(*this, &AttachHddDialog::onSrChanged));
    _vdiList.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &AttachHddDialog::onVdiChanged));
    _addButton.signal_clicked().connect(sigc::mem_fun(*this, &AttachHddDialog::onAdd));
    _resizeButton.signal_clicked().connect(sigc::mem_fun(*this, &AttachHddDialog::onResize));
    _removeButton.signal_clicked().connect(sigc::mem_fun(*this, &AttachHddDialog::onRemove));

    _srCombo.selectDefault();

    show_all_children();

    onVdiChanged();
}


AttachHddDialog::~AttachHddDialog()
{
    _srUpdated.disconnect();
}


void AttachHddDialog::onSrChanged()
{
    const Session& session = _vm.getSession();
    Glib::ustring refid = _srCombo.getSelected();
    RefPtr<StorageRepository> sr = session.getStore().getSr(refid);
    if (sr)
    {
        update(*sr);
        _srUpdated.disconnect();
        _srUpdated = SignalManager::instance().xenObjectSignal(*sr).connect(sigc::mem_fun(*this, &AttachHddDialog::onObjectUpdated));
    }
    else
    {
        _vdiList.update(session, NULL, VirtualDiskImageListView::ATTACHABLE_ONLY);
    }
}


void AttachHddDialog::onObjectUpdated(RefPtr<XenObject> object, int what)
{
    update((StorageRepository&)*object);
}


void AttachHddDialog::update(const StorageRepository& sr)
{
    const Session& session = sr.getSession();
    XenPtr<xen_sr_record> record = sr.getRecord();
    _vdiList.update(session, record->vdis, VirtualDiskImageListView::ATTACHABLE_ONLY);
}


void AttachHddDialog::onVdiChanged()
{
    Glib::ustring refid = _vdiList.getSelected();
    bool sensitivity = refid.empty() ? false : true;
    _pApplyButton->set_sensitive(sensitivity);
    _resizeButton.set_sensitive(sensitivity);
    _removeButton.set_sensitive(sensitivity);
}


void AttachHddDialog::onAdd()
{
    const Session& session = _vm.getSession();
    Glib::ustring refid = _srCombo.getSelected();
    RefPtr<StorageRepository> sr = session.getStore().getSr(refid);
    if (sr)
    {
        Controller::instance().addHddTo(*sr);
    }
}


void AttachHddDialog::onResize()
{
    const Session& session = _vm.getSession();
    Glib::ustring refid = _vdiList.getSelected();
    RefPtr<VirtualDiskImage> vdi = session.getStore().getVdi(refid);
    if (vdi)
    {
        Controller::instance().resizeVdi(*vdi);
    }
}


void AttachHddDialog::onRemove()
{
    const Session& session = _vm.getSession();
    Glib::ustring refid = _vdiList.getSelected();
    RefPtr<VirtualDiskImage> vdi = session.getStore().getVdi(refid);
    if (vdi)
    {
        Controller::instance().removeVdi(*vdi);
    }
}


Glib::ustring AttachHddDialog::getUserDevice()
{
    return _devCombo.getSelected();
}


Glib::ustring AttachHddDialog::getVdi()
{
    return _vdiList.getSelected();
}
