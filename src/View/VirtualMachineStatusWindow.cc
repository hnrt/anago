// Copyright (C) 2012-2017 Hideaki Narita


//#define NO_TRACE


#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include "Base/StringBuffer.h"
#include "File/File.h"
#include "Logger/Trace.h"
#include "Controller/SignalManager.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/VirtualMachineExporter.h"
#include "XenServer/VirtualMachineImporter.h"
#include "PixStore.h"
#include "VirtualMachineStatusWindow.h"


using namespace hnrt;


VirtualMachineStatusWindow::VirtualMachineStatusWindow(Gtk::Window& parent)
    : _buttonBox(Gtk::BUTTONBOX_END, 6)
    , _timeoutInUse(0)
{
    set_title(gettext("Virtual machine - Import / Export / Verify"));
    set_default_size(450, 200);
    set_icon(PixStore::instance().getApp());

    add(_vbox);

    _vbox.set_spacing(6);
    _vbox.pack_start(_listViewSw, Gtk::PACK_EXPAND_WIDGET);
    _vbox.pack_start(_buttonBox, Gtk::PACK_SHRINK);

    _listViewSw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _listViewSw.set_shadow_type(Gtk::SHADOW_IN);
    _listViewSw.add(_listView);

    _buttonBox.pack_start(_cancelButton);
    _buttonBox.pack_start(_dismissButton);

    _cancelButton.set_label(gettext("_Cancel"));
    _cancelButton.set_use_underline(true);
    _dismissButton.set_label(gettext("_Dismiss"));
    _dismissButton.set_use_underline(true);

    show_all_children();

    Glib::RefPtr<Gtk::TreeSelection> selection = _listView.get_selection();
    selection->signal_changed().connect(sigc::mem_fun(*this, &VirtualMachineStatusWindow::onSelectionChanged));

    _cancelButton.signal_clicked().connect(sigc::mem_fun(*this, &VirtualMachineStatusWindow::onCancel));
    _dismissButton.signal_clicked().connect(sigc::mem_fun(*this, &VirtualMachineStatusWindow::onDismiss));

    updateButtonSensitivity();

    SignalManager::instance().xenObjectSignal(XenObject::CREATED).connect(sigc::mem_fun(*this, &VirtualMachineStatusWindow::onObjectCreated));

}


VirtualMachineStatusWindow::~VirtualMachineStatusWindow()
{
    if (_timeoutInUse > 0)
    {
        _timeout.disconnect();
    }

    _listView.clear();
}


void VirtualMachineStatusWindow::onObjectCreated(RefPtr<XenObject> object, int what)
{
    switch (object->getType())
    {
    case XenObject::VM_EXPORTER:
    case XenObject::VM_IMPORTER:
    {
        SignalManager::instance().xenObjectSignal(*object).connect(sigc::mem_fun(*this, &VirtualMachineStatusWindow::onObjectUpdated));
        _listView.add(object);
        updateButtonSensitivity();
        show();
        if (_timeoutInUse++ == 0)
        {
            _timeout = Glib::signal_timeout().connect(sigc::mem_fun(*this, &VirtualMachineStatusWindow::onTimedOut), 1000);
        }
        break;
    }
    default:
        break;
    }
}


void VirtualMachineStatusWindow::onObjectUpdated(RefPtr<XenObject> object, int what)
{
    if (what == XenObject::DESTROYED)
    {
        finishCancellation(object->getUUID());
        _listView.remove(object);
        if (--_timeoutInUse == 0)
        {
            _timeout.disconnect();
        }
    }
    else
    {
        _listView.update(object);
    }
    updateButtonSensitivity();
}


bool VirtualMachineStatusWindow::onTimedOut()
{
    TRACE("VirtualMachineStatusWindow::onTimedOut");
    _listView.update();
    return true; // to be invoked again
}


void VirtualMachineStatusWindow::onSelectionChanged()
{
    updateButtonSensitivity();
}


void VirtualMachineStatusWindow::updateButtonSensitivity()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _listView.get_selection();
    Gtk::TreeIter iter = selection->get_selected();
    if (iter)
    {
        VirtualMachineOperationState state = _listView.getState(iter);
        if (state.isInactive())
        {
            _cancelButton.set_sensitive(false);
            _dismissButton.set_sensitive(true);
        }
        else if (state.isCanceling())
        {
            _cancelButton.set_sensitive(false);
            _dismissButton.set_sensitive(false);
        }
        else
        {
            _cancelButton.set_sensitive(true);
            _dismissButton.set_sensitive(false);
        }
    }
    else
    {
        _cancelButton.set_sensitive(false);
        _dismissButton.set_sensitive(false);
    }
}


void VirtualMachineStatusWindow::onCancel()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _listView.get_selection();
    Gtk::TreeIter iter = selection->get_selected();
    if (iter)
    {
        Glib::ustring uuid = _listView.cancel(iter);
        if (!uuid.empty())
        {
            cancel(uuid);
        }
        updateButtonSensitivity();
    }
}


void VirtualMachineStatusWindow::onDismiss()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _listView.get_selection();
    Gtk::TreeIter iter = selection->get_selected();
    if (iter)
    {
        _listView.erase(iter);
    }
}


void VirtualMachineStatusWindow::cancel(const Glib::ustring& uuid)
{
    Glib::Mutex::Lock lock(_mutex);
    for (std::list<Glib::ustring>::iterator iter = _canceling.begin(); iter != _canceling.end(); iter++)
    {
        if (uuid == *iter)
        {
            return;
        }
    }
    _canceling.push_back(uuid);
}


void VirtualMachineStatusWindow::finishCancellation(const Glib::ustring& uuid)
{
    Glib::Mutex::Lock lock(_mutex);
    for (std::list<Glib::ustring>::iterator iter = _canceling.begin(); iter != _canceling.end(); iter++)
    {
        if (uuid == *iter)
        {
            _canceling.erase(iter);
            return;
        }
    }
}


bool VirtualMachineStatusWindow::canContinue(const Glib::ustring& uuid)
{
    Glib::Mutex::Lock lock(_mutex);
    for (std::list<Glib::ustring>::iterator iter = _canceling.begin(); iter != _canceling.end(); iter++)
    {
        if (uuid == *iter)
        {
            return false;
        }
    }
    return true;
}


VirtualMachineStatusWindow::ListView::ListView()
{
    _store = Gtk::ListStore::create(_record);
    set_model(_store);
    append_column(gettext("Status"), _record.colDisplayState);
    append_column(gettext("Elapsed time"), _record.colElapsedTime);
    append_column(gettext("File size"), _record.colDisplaySize);
    append_column(gettext("Virtual machine"), _record.colName);
    append_column(gettext("File name"), _record.colPath);
    get_column(0)->set_resizable(true);
    get_column(0)->set_reorderable(false);
    get_column(1)->set_resizable(true);
    get_column(1)->set_reorderable(false);
    get_column(2)->set_resizable(true);
    get_column(2)->set_reorderable(false);
    get_column(3)->set_resizable(true);
    get_column(3)->set_reorderable(false);
    get_column(4)->set_resizable(true);
    get_column(4)->set_reorderable(false);
    set_rules_hint(true);
    Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
    selection->set_mode(Gtk::SELECTION_SINGLE);
}


void VirtualMachineStatusWindow::ListView::clear()
{
    _store->clear();
}


void VirtualMachineStatusWindow::ListView::add(RefPtr<XenObject> object)
{
    TRACE("VirtualMachineStatusWindow::ListView::add", "object=\"%s\"", object->getName().c_str());
    Glib::ustring uuid = object->getUUID();
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    Gtk::TreeModel::Row row;
    for (;;)
    {
        if (!iter)
        {
            row = *_store->append();
            row[_record.colId] = uuid;
            break;
        }
        row = *iter;
        Glib::ustring current = row[_record.colId];
        if (current == uuid)
        {
            break;
        }
        iter++;
    }
    row[_record.colOperator] = object;
    row[_record.colStartTime] = time(NULL);
    update(row);
}


void VirtualMachineStatusWindow::ListView::update(RefPtr<XenObject> object)
{
    Glib::ustring uuid = object->getUUID();
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring current = row[_record.colId];
        if (current == uuid)
        {
            update(row);
            return;
        }
        iter++;
    }
}


void VirtualMachineStatusWindow::ListView::remove(RefPtr<XenObject> object)
{
    TRACE("VirtualMachineStatusWindow::ListView::remove", "object=\"%s\"", object->getName().c_str());
    Glib::ustring uuid = object->getUUID();
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring current = row[_record.colId];
        if (current == uuid)
        {
            update(row);
            row[_record.colOperator] = RefPtr<XenObject>();
            return;
        }
        iter++;
    }
}


void VirtualMachineStatusWindow::ListView::update()
{
    time_t currentTime = time(NULL);
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        VirtualMachineOperationState state = row[_record.colState];
        if (state.isActive())
        {
            updateTime(row, currentTime);
        }
        iter++;
    }
}


bool VirtualMachineStatusWindow::ListView::on_button_press_event(GdkEventButton* event)
{
    bool retval = Gtk::TreeView::on_button_press_event(event);
    return retval;
}


VirtualMachineOperationState VirtualMachineStatusWindow::ListView::getState(Gtk::TreeIter iter)
{
    Gtk::TreeModel::Row row = *iter;
    return row[_record.colState];
}


Glib::ustring VirtualMachineStatusWindow::ListView::cancel(Gtk::TreeIter iter)
{
    Glib::ustring uuid;
    Gtk::TreeModel::Row row = *iter;
    VirtualMachineOperationState state = row[_record.colState];
    state = state.canceling();
    if (state.isCanceling())
    {
        RefPtr<XenObject> object = row[_record.colOperator];
        if (!object)
        {
            Logger::instance().error("VirtualMachineStatusWindow::ListView::cancel: Already removed. name=\"%s\"",
                                     ((Glib::ustring)row[_record.colName]).c_str());
        }
        else if (object->getType() == XenObject::VM_EXPORTER)
        {
            VirtualMachineExporter& exporter = (VirtualMachineExporter&)*object;
            exporter.abort();
            updateState(row, state);
            updateTime(row, time(NULL));
            uuid = row[_record.colId];
        }
        else if (object->getType() == XenObject::VM_IMPORTER)
        {
            VirtualMachineImporter& importer = (VirtualMachineImporter&)*object;
            importer.abort();
            updateState(row, state);
            updateTime(row, time(NULL));
            uuid = row[_record.colId];
        }
    }
    return uuid;
}


void VirtualMachineStatusWindow::ListView::erase(Gtk::TreeIter iter)
{
    _store->erase(iter);
}


void VirtualMachineStatusWindow::ListView::update(Gtk::TreeModel::Row& row)
{
    RefPtr<XenObject> object = row[_record.colOperator];
    if (!object)
    {
        return;
    }
    Glib::ustring name = row[_record.colName];
    Glib::ustring path = row[_record.colPath];
    int64_t size = -1;
    VirtualMachineOperationState state;
    int percent = -1;
    time_t currentTime = time(NULL);
    switch (object->getType())
    {
    case XenObject::VM_EXPORTER:
    {
        VirtualMachineExporter& exporter = (VirtualMachineExporter&)*object;
        RefPtr<VirtualMachine> vm = exporter.vm();
        if (vm)
        {
            name = vm->getName();
        }
        if (path.empty())
        {
            const char* p = exporter.path();
            if (p)
            {
                row[_record.colPath] = Glib::ustring(p);
            }
        }
        state = exporter.state();
        if (state == VirtualMachineOperationState::EXPORT_VERIFY_INPROGRESS ||
            state == VirtualMachineOperationState::EXPORT_VERIFY_PENDING)
        {
            percent = exporter.percent();
        }
        else
        {
            size = exporter.nbytes();
        }
        break;
    }
    case XenObject::VM_IMPORTER:
    {
        VirtualMachineImporter& importer = (VirtualMachineImporter&)*object;
        RefPtr<VirtualMachine> vm = importer.vm();
        if (vm)
        {
            name = vm->getName();
        }
        if (path.empty())
        {
            const char* p = importer.path();
            if (p)
            {
                row[_record.colPath] = Glib::ustring(p);
            }
        }
        if (!(int64_t)row[_record.colSize])
        {
            size = importer.size();
        }
        state = importer.state();
        percent = importer.percent();
        break;
    }
    default:
        break;
    }
    row[_record.colName] = name;
    if (!((VirtualMachineOperationState)row[_record.colState]).isCanceling() || state.isInactive())
    {
        if (size >= 0)
        {
            updateSize(row, size);
        }
        if (state == VirtualMachineOperationState::IMPORT_INPROGRESS ||
            state == VirtualMachineOperationState::IMPORT_PENDING ||
            state == VirtualMachineOperationState::EXPORT_VERIFY_INPROGRESS ||
            state == VirtualMachineOperationState::EXPORT_VERIFY_PENDING ||
            state == VirtualMachineOperationState::VERIFY_INPROGRESS ||
            state == VirtualMachineOperationState::VERIFY_PENDING)
        {
            updateState(row, state, percent);
        }
        else
        {
            updateState(row, state);
        }
    }
    updateTime(row, currentTime);
}


void VirtualMachineStatusWindow::ListView::updateSize(Gtk::TreeModel::Row& row, int64_t size)
{
    row[_record.colSize] = size;
    StringBuffer displaySize;
    displaySize.format("%'zu", size);
    row[_record.colDisplaySize] = Glib::ustring(displaySize.str());
}


static const char* getStateText(VirtualMachineOperationState state)
{
    switch (state)
    {
    case VirtualMachineOperationState::IMPORT_PENDING:
    case VirtualMachineOperationState::IMPORT_INPROGRESS:
        return gettext("Importing...");
    case VirtualMachineOperationState::IMPORT_SUCCESS:
        return gettext("Imported");
    case VirtualMachineOperationState::IMPORT_FAILURE:
        return gettext("Import failed");
    case VirtualMachineOperationState::IMPORT_CANCELING:
        return gettext("Canceling import...");
    case VirtualMachineOperationState::IMPORT_CANCELED:
        return gettext("Import canceled");
    case VirtualMachineOperationState::EXPORT_PENDING:
    case VirtualMachineOperationState::EXPORT_INPROGRESS:
        return gettext("Exporting...");
    case VirtualMachineOperationState::EXPORT_SUCCESS:
        return gettext("Exported");
    case VirtualMachineOperationState::EXPORT_FAILURE:
        return gettext("Export failed");
    case VirtualMachineOperationState::EXPORT_CANCELING:
        return gettext("Canceling export...");
    case VirtualMachineOperationState::EXPORT_CANCELED:
        return gettext("Export canceled");
    case VirtualMachineOperationState::EXPORT_VERIFY_PENDING:
    case VirtualMachineOperationState::EXPORT_VERIFY_INPROGRESS:
        return gettext("Verifying...");
    case VirtualMachineOperationState::EXPORT_VERIFY_SUCCESS:
        return gettext("Exported and verified");
    case VirtualMachineOperationState::EXPORT_VERIFY_FAILURE:
        return gettext("Exported but verify failed");
    case VirtualMachineOperationState::EXPORT_VERIFY_CANCELING:
        return gettext("Canceling verify...");
    case VirtualMachineOperationState::EXPORT_VERIFY_CANCELED:
        return gettext("Exported but verify canceled");
    case VirtualMachineOperationState::VERIFY_PENDING:
    case VirtualMachineOperationState::VERIFY_INPROGRESS:
        return gettext("Verifying...");
    case VirtualMachineOperationState::VERIFY_SUCCESS:
        return gettext("Verified");
    case VirtualMachineOperationState::VERIFY_FAILURE:
        return gettext("Verify failed");
    case VirtualMachineOperationState::VERIFY_CANCELING:
        return gettext("Canceling verify...");
    case VirtualMachineOperationState::VERIFY_CANCELED:
        return gettext("Verify canceled");
    default:
        return "?";
    }
}


void VirtualMachineStatusWindow::ListView::updateState(Gtk::TreeModel::Row& row, VirtualMachineOperationState state, int percent)
{
    row[_record.colState] = state;
    StringBuffer displayState;
    displayState = getStateText(state);
    if (percent >= 0)
    {
        displayState.appendFormat("(%d%%)", percent);
    }
    row[_record.colDisplayState] = Glib::ustring(displayState.str());
    RefPtr<XenObject> object  = row[_record.colOperator];
    switch (object->getType())
    {
    case XenObject::VM_EXPORTER:
    case XenObject::VM_IMPORTER:
    {
        VirtualMachinePorter& porter = (VirtualMachinePorter&)*object;
        RefPtr<VirtualMachine> vm = porter.vm();
        if (vm)
        {
            if (state.isActive())
            {
                vm->setDisplayStatus(displayState);
            }
            else
            {
                XenPtr<xen_vm_record> record = vm->getRecord();
                vm->setDisplayStatus(XenServer::getPowerStateText(record->power_state));
            }
        }
        break;
    }
    default:
        break;
    }
}


void VirtualMachineStatusWindow::ListView::updateTime(Gtk::TreeModel::Row& row, time_t curTime)
{
    time_t elapsed = curTime;
    elapsed -= row[_record.colStartTime];
    int seconds = static_cast<int>(elapsed % 60);
    elapsed /= 60;
    int minutes = static_cast<int>(elapsed % 60);
    elapsed /= 60;
    int hours = static_cast<int>(elapsed);
    StringBuffer displayTime;
    if (hours)
    {
        displayTime.format("%d:%02d:%02d", hours, minutes, seconds);
    }
    else
    {
        displayTime.format("%02d:%02d", minutes, seconds);
    }
    row[_record.colElapsedTime] = Glib::ustring(displayTime.str());
}
