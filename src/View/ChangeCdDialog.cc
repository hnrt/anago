// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "XenServer/VirtualBlockDevice.h"
#include "XenServer/VirtualMachine.h"
#include "ChangeCdDialog.h"


using namespace hnrt;


ChangeCdDialog::ChangeCdDialog(Gtk::Window& parent, const VirtualMachine& vm)
    : Gtk::Dialog(gettext("Change CD/DVD"), parent)
    , _devLabel(gettext("Device:"))
    , _devCombo(vm)
    , _imgLabel(gettext("Virtual disk image:"))
    , _imgListView(vm.getSession(), true)
{
    set_default_size(600, 300);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    _pApplyButton = add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();

    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _table.attach(_devLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_devBox, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_imgLabel, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
    _table.attach(*Gtk::manage(_imgListView.createScrolledWindow()), 1, 2, 1, 2);

    _devLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _imgLabel.set_alignment(1.0, 0.0); // h=right, v=top

    _devBox.pack_start(_devCombo, Gtk::PACK_SHRINK);

    _devCombo.signal_changed().connect(sigc::mem_fun(*this, &ChangeCdDialog::onDeviceChanged));

    show_all_children();

    onDeviceChanged();
    onImageChanged();

    _imgListView.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &ChangeCdDialog::onImageChanged));
}


void ChangeCdDialog::select(const Glib::ustring& refid)
{
    _devCombo.select(refid);
}


void ChangeCdDialog::onDeviceChanged()
{
    Glib::ustring vdi = _devCombo.getSelectedImage();
    _imgListView.select(vdi);
}


void ChangeCdDialog::onImageChanged()
{
    Glib::ustring selected = _imgListView.getSelected();
    _pApplyButton->set_sensitive(!selected.empty());
}
