// Copyright (C) 2012-2018 Hideaki Narita


#include <libintl.h>
#include "Base/StringBuffer.h"
#include "XenServer/Session.h"
#include "XenServer/XenServer.h"
#include "HardDiskDriveListBox.h"
#include "HardDiskDriveSpecDialog.h"


using namespace hnrt;


HardDiskDriveListBox::HardDiskDriveListBox(Gtk::Window& parent, const Session& session)
    : _parent(parent)
    , _listView(session)
    , _session(session)
{
    set_spacing(6);

    _listView.signalChanged().connect(sigc::mem_fun(*this, &HardDiskDriveListBox::onListViewChanged));
    _listView.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &HardDiskDriveListBox::onSelectionChanged));
    pack_start(*Gtk::manage(_listView.createScrolledWindow()));

    pack_start(_buttonsBox, Gtk::PACK_SHRINK);

    _addButton.set_label(gettext("Add"));
    _buttonsBox.pack_start(_addButton, Gtk::PACK_SHRINK);

    _editButton.set_label(gettext("Edit"));
    _buttonsBox.pack_start(_editButton, Gtk::PACK_SHRINK);

    _removeButton.set_label(gettext("Remove"));
    _buttonsBox.pack_start(_removeButton, Gtk::PACK_SHRINK);

    _addButton.set_sensitive(false);
    _editButton.set_sensitive(false);
    _removeButton.set_sensitive(false);

    _addButton.signal_clicked().connect(sigc::mem_fun(*this, &HardDiskDriveListBox::onAdd));
    _editButton.signal_clicked().connect(sigc::mem_fun(*this, &HardDiskDriveListBox::onEdit));
    _removeButton.signal_clicked().connect(sigc::mem_fun(*this, &HardDiskDriveListBox::onRemove));
}


void HardDiskDriveListBox::onListViewChanged()
{
    if (_listView.getCount())
    {
        _addButton.set_sensitive(true);
    }
    else
    {
        _addButton.set_sensitive(false);
    }
}


void HardDiskDriveListBox::onSelectionChanged()
{
    int i = _listView.getSelected();
    if (i < 0)
    {
        _editButton.set_sensitive(false);
        _removeButton.set_sensitive(false);
    }
    else if (i == 0)
    {
        _editButton.set_sensitive(true);
        _removeButton.set_sensitive(false);
    }
    else
    {
        _editButton.set_sensitive(true);
        _removeButton.set_sensitive(true);
    }
}


void HardDiskDriveListBox::onAdd()
{
    HardDiskDriveSpecDialog dialog(_parent, _session, gettext("Add hard disk drive to SR"));
    HardDiskDriveSpec spec;
    spec.srREFID = XenServer::getDefaultSr(_session);
    spec.size = 1024L * 1024L * 1024L;
    spec.label = StringBuffer().format(gettext("Hard disk drive %d"), _listView.getCount());
    spec.description = gettext("Created by Anago");
    dialog.setValue(spec);
    int response = dialog.run();
    if (response != Gtk::RESPONSE_APPLY)
    {
        return;
    }
    dialog.getValue(spec);
    _listView.addValue(spec);
}


void HardDiskDriveListBox::onEdit()
{
    int index = _listView.getSelected();
    HardDiskDriveSpec spec;
    _listView.getValue(index, spec);
    HardDiskDriveSpecDialog dialog(_parent, _session, gettext("Edit hard disk drive"));
    dialog.setValue(spec);
    int response = dialog.run();
    if (response != Gtk::RESPONSE_APPLY)
    {
        return;
    }
    dialog.getValue(spec);
    _listView.setValue(index, spec);
}


void HardDiskDriveListBox::onRemove()
{
    int index = _listView.getSelected();
    HardDiskDriveSpec spec;
    _listView.getValue(index, spec);
    Gtk::MessageDialog dialog(_parent,
                              Glib::ustring::compose(
                                  gettext("Are you sure to remove the hard disk drive \"%1\"?"),
                                  spec.label),
                              false,
                              Gtk::MESSAGE_WARNING,
                              Gtk::BUTTONS_YES_NO,
                              true);
    int response = dialog.run();
    if (response != Gtk::RESPONSE_YES)
    {
        return;
    }
    _listView.removeValue(index);
}
