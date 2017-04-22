// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "XenServer/Session.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/VirtualMachineSpec.h"
#include "XenServer/XenObjectStore.h"
#include "XenServer/XenServer.h"
#include "AddVmDialog.h"


using namespace hnrt;


AddVmDialog::AddVmDialog(Gtk::Window& parent, const Session& session)
    : Gtk::Dialog(gettext("Add new VM"), parent)
    , _templateLv(session)
    , _teSw(*Gtk::manage(_templateLv.createScrolledWindow()))
    , _hddBox(parent, session)
    , _cdLv(session, true)
    , _cdSw(*Gtk::manage(_cdLv.createScrolledWindow()))
    , _nwLv(session)
    , _nwSw(*Gtk::manage(_nwLv.createScrolledWindow()))
    , _session(session)
{
    set_default_size(600, 400);
    set_border_width(6);

    _pCancelButton = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    _pApplyButton = add_button(Gtk::Stock::OK, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();
    box->set_spacing(6);
    box->pack_start(_hpaned);
    _hpaned.pack1(_box1);
    _hpaned.pack2(_box2);

    _templateLabel.set_text(gettext("Template:"));
    _templateLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _box1.pack_start(_templateLabel, Gtk::PACK_SHRINK);

    _templateLv.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &AddVmDialog::onTemplateChanged));
    _box1.pack_start(_teSw);

    _nameLabel.set_text(gettext("Name:"));
    _nameLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _box1.pack_start(_nameLabel, Gtk::PACK_SHRINK);

    _nameEntry.set_text(gettext("New virtual machine"));
    _box1.pack_start(_nameEntry, Gtk::PACK_SHRINK);

    _descLabel.set_text(gettext("Description:"));
    _descLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _box1.pack_start(_descLabel, Gtk::PACK_SHRINK);

    _descEntry.set_text(gettext("Created by Anago"));
    _box1.pack_start(_descEntry, Gtk::PACK_SHRINK);

    _hddLabel.set_text(gettext("Hard disk drives:"));
    _hddLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _box1.pack_start(_hddLabel, Gtk::PACK_SHRINK);

    _hddBox.listView().signalChanged().connect(sigc::mem_fun(*this, &AddVmDialog::onHardDiskDriveChanged));
    _box1.pack_start(_hddBox);

    _cdLabel.set_text(gettext("Installation disk in CD/DVD drive:"));
    _cdLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _box2.pack_start(_cdLabel, Gtk::PACK_SHRINK);

    _cdLv.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &AddVmDialog::onCdChanged));
    _box2.pack_start(_cdSw);

    _nicLabel.set_text(gettext("Network interface cards:"));
    _nicLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _box2.pack_start(_nicLabel, Gtk::PACK_SHRINK);

    _nwLv.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &AddVmDialog::onNetworkChanged));
    _box2.pack_start(_nwSw);

    show_all_children();

    _pCancelButton->grab_focus();
    _templateLv.get_selection()->unselect_all();
    _nameEntry.set_sensitive(false);
    _descEntry.set_sensitive(false);
    _hddBox.set_sensitive(false);
    _cdSw.set_sensitive(false);
    _nwSw.set_sensitive(false);
    _pApplyButton->set_sensitive(false);
}


void AddVmDialog::onTemplateChanged()
{
    _hddBox.listView().clear();
    RefPtr<VirtualMachine> vm;
    Glib::ustring refid = _templateLv.getSelected();
    if (!refid.empty())
    {
        vm = _session.getStore().getVm(refid);
    }

    if (!vm)
    {
        _nameEntry.set_sensitive(false);
        _descEntry.set_sensitive(false);
        _hddBox.set_sensitive(false);
        _cdSw.set_sensitive(false);
        _nwSw.set_sensitive(false);
        _pApplyButton->set_sensitive(false);
        return;
    }

    _nameEntry.set_sensitive(true);
    _descEntry.set_sensitive(true);
    _hddBox.set_sensitive(true);
    _cdSw.set_sensitive(true);
    _nwSw.set_sensitive(true);

    HardDiskDriveSpec hddSpec;
    hddSpec.size = XenServer::getDiskSizeHint(vm->getRecord());
    hddSpec.srREFID = _session.getStore().getSrCandidate(hddSpec.size, XenServer::getDefaultSr(_session));
    hddSpec.name = gettext("Hard disk drive 0");
    hddSpec.description = gettext("Created by Anago");
    _hddBox.listView().addValue(hddSpec);

    validate();
}


void AddVmDialog::onHardDiskDriveChanged()
{
    validate();
}


void AddVmDialog::onCdChanged()
{
    validate();
}


void AddVmDialog::onNetworkChanged()
{
    validate();
}


void AddVmDialog::validate()
{
    _pApplyButton->set_sensitive(false);

    if (!_hddBox.listView().getCount())
    {
        return;
    }

    Glib::ustring cd = _cdLv.getSelected();
    if (cd.empty())
    {
        return;
    }

    std::list<Glib::ustring> nwList;
    if (!_nwLv.getSelected(nwList))
    {
        return;
    }

    _pApplyButton->set_sensitive(true);
}


int AddVmDialog::getHardDiskDriveList(std::list<HardDiskDriveSpec>& list) const
{
    int n = _hddBox.listView().getCount();
    for (int i = 0; i < n; i++)
    {
        HardDiskDriveSpec spec;
        _hddBox.listView().getValue(i, spec);
        list.push_back(spec);
    }
    return n;
}


void AddVmDialog::getSpec(VirtualMachineSpec& spec)
{
    spec.templateREFID = getTemplate();
    spec.name = getName();
    spec.desc = getDesc();
    getHardDiskDriveList(spec.hddList);
    spec.cdREFID = getCd();
    getNetworks(spec.nwList);
}
