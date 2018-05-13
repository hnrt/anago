// Copyright (C) 2012-2018 Hideaki Narita


#include <libintl.h>
#include <stdio.h>
#include <string.h>
#include "Base/StringBuffer.h"
#include "Controller/SignalManager.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Model/PatchBase.h"
#include "XenServer/Host.h"
#include "XenServer/Patch.h"
#include "XenServer/Session.h"
#include "XenServer/XenObject.h"
#include "PatchListView.h"
#include "View.h"


using namespace hnrt;


PatchListView::PatchListView(const Host& host)
    : _host(host)
{
    _store = Gtk::ListStore::create(_record);
    set_model(_store);
    append_column(gettext("UUID"), _record.colId);
    append_column(gettext("Label"), _record.colLabel);
    append_column(gettext("Description"), _record.colDescription);
    append_column(gettext("Status"), _record.colDisplayStatus);
    append_column(gettext("After apply"), _record.colAfterApplyGuidance);
    append_column(gettext("Required"), _record.colRequired);
    append_column(gettext("Conflicting"), _record.colConflicting);
    append_column(gettext("Timestamp"), _record.colDisplayTimestamp);
    append_column(gettext("Size"), _record.colDisplaySize);
    append_column(gettext("Version"), _record.colVersion);
    append_column(gettext("URL"), _record.colUrl);
    append_column(gettext("Patch URL"), _record.colPatchUrl);
    for (int i = 0; i < 12; i++)
    {
        get_column(i)->set_resizable(true);
        get_column(i)->set_reorderable(true);
    }
    set_rules_hint(true);
    Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
    selection->set_mode(Gtk::SELECTION_SINGLE);
    SignalManager::instance().xenObjectSignal(XenObject::PATCH_DOWNLOAD_PENDING).connect(sigc::mem_fun(*this, &PatchListView::onNotify));
    SignalManager::instance().xenObjectSignal(XenObject::PATCH_UPLOAD_PENDING).connect(sigc::mem_fun(*this, &PatchListView::onNotify));
    SignalManager::instance().xenObjectSignal(XenObject::PATCH_APPLY_PENDING).connect(sigc::mem_fun(*this, &PatchListView::onNotify));
}


void PatchListView::clear()
{
    _store->clear();
}


static const char* GetDisplayStatus(PatchState status)
{
    switch (status)
    {
    case PatchState::AVAILABLE: return gettext("Available");
    case PatchState::DOWNLOAD_PENDING: return gettext("Downloading...");
    case PatchState::DOWNLOAD_INPROGRESS: return gettext("Downloading...");
    case PatchState::DOWNLOAD_FAILURE: return gettext("Download failed");
    case PatchState::DOWNLOADED: return gettext("Downloaded");
    case PatchState::UPLOAD_PENDING: return gettext("Uploading...");
    case PatchState::UPLOAD_INPROGRESS: return gettext("Uploading...");
    case PatchState::UPLOAD_FAILURE: return gettext("Upload failed");
    case PatchState::UPLOADED: return gettext("Uploaded");
    case PatchState::APPLY_INPROGRESS: return gettext("Applying...");
    case PatchState::APPLY_FAILURE: return gettext("Apply failed");
    case PatchState::APPLIED: return gettext("Applied");
    case PatchState::CLEAN_INPROGRESS: return gettext("Cleaning...");
    case PatchState::CLEAN_FAILURE: return gettext("Clean failed");
    default: return "";
    }
}


static Glib::ustring GetLabelList(const std::list<Glib::ustring>& list)
{
    StringBuffer sb;
    RefPtr<PatchBase> pb = Model::instance().getPatchBase();
    if (pb)
    {
        for (std::list<Glib::ustring>::const_iterator i = list.begin(); i != list.end(); i++)
        {
            RefPtr<PatchRecord> record = pb->getRecord(*i);
            if (record)
            {
                if (sb.len())
                {
                    sb += "\n";
                }
                sb += record->label.c_str();
            }
        }
    }
    return Glib::ustring(sb.str());
}


static Glib::ustring GetDisplayTimestamp(time_t t)
{
    StringBuffer retval;
    if (t)
    {
        struct tm x = { 0 };
        gmtime_r(&t, &x);
        //retval.format("%04d-%02d-%02d %02d:%02d:%02d", x.tm_year + 1900, x.tm_mon + 1, x.tm_mday, x.tm_hour, x.tm_min, x.tm_sec);
        retval.format("%04d-%02d-%02d", x.tm_year + 1900, x.tm_mon + 1, x.tm_mday);
    }
    return Glib::ustring(retval);
}


void PatchListView::set(const PatchRecord& patchRecord)
{
    Gtk::TreeModel::Row row;
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        row = *iter;
        if (row[_record.colId] == patchRecord.uuid)
        {
            goto done;
        }
        iter++;
    }
    iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        row = *iter;
        if (strcasecmp(patchRecord.label.c_str(), ((Glib::ustring)row[_record.colLabel]).c_str()) < 0)
        {
            row = *_store->insert(iter);
            row[_record.colId] = patchRecord.uuid;
            goto done;
        }
        iter++;
    }
    row = *_store->append();
    row[_record.colId] = patchRecord.uuid;
done:
    row[_record.colLabel] = patchRecord.label;
    row[_record.colDescription] = patchRecord.description;
    row[_record.colStatus] = patchRecord.state;
    row[_record.colDisplayStatus] = GetDisplayStatus(patchRecord.state);
    row[_record.colAfterApplyGuidance] = patchRecord.afterApplyGuidance;
    row[_record.colRequired] = GetLabelList(patchRecord.required);
    row[_record.colConflicting] = GetLabelList(patchRecord.conflicting);
    row[_record.colTimestamp] = patchRecord.timestamp;
    row[_record.colDisplayTimestamp] = GetDisplayTimestamp(patchRecord.timestamp);
    row[_record.colSize] = patchRecord.size;
    row[_record.colDisplaySize] = Glib::ustring(patchRecord.size ? StringBuffer().format("%'zu", patchRecord.size).str() : "");
    row[_record.colVersion] = patchRecord.version;
    row[_record.colUrl] = patchRecord.url;
    row[_record.colPatchUrl] = patchRecord.patchUrl;
}


bool PatchListView::on_button_press_event(GdkEventButton* event)
{
    bool retval = Gtk::TreeView::on_button_press_event(event);

    if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
    {
        Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
        Gtk::TreeIter iter = selection->get_selected();
        if (iter)
        {
            Gtk::TreeModel::Row row = *iter;
            Glib::ustring uuid = row[_record.colId];
            _menu.popup(event->button, event->time, uuid.c_str(), row[_record.colStatus]);
            // The event has been handled.
            return true;
        }
    }

    return retval;
}


Gtk::ScrolledWindow* PatchListView::createScrolledWindow()
{
    Gtk::ScrolledWindow* pW = new Gtk::ScrolledWindow();
    pW->add(*this);
    pW->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    pW->set_shadow_type(Gtk::SHADOW_IN);
    return pW;
}


void PatchListView::onNotify(RefPtr<XenObject> object, int what)
{
    RefPtr<Patch> patch = RefPtr<Patch>::castStatic(object);
    if (patch->getSession() != _host.getSession())
    {
        return;
    }
    TRACEFUN(this, "PatchListView::onNotify(%s)", GetNotificationText(what));
    RefPtr<PatchRecord> patchRecord = patch->getRecord();
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        if (row[_record.colId] == patchRecord->uuid)
        {
            row[_record.colStatus] = patchRecord->state;
            StringBuffer buf;
            buf = GetDisplayStatus(patchRecord->state);
            if (what == XenObject::PATCH_DOWNLOADING ||
                what == XenObject::PATCH_UPLOADING)
            {
                size_t n = patch->getExpected();
                size_t m = patch->getActual();
                if (n)
                {
                    buf.appendFormat("(%zu%%)", (100 * m) / n);
                }
                else
                {
                    buf.appendFormat("(%'zu)", m);
                }
            }
            row[_record.colDisplayStatus] = Glib::ustring(buf.str());
            switch (what)
            {
            case XenObject::PATCH_DOWNLOAD_PENDING:
            case XenObject::PATCH_UPLOAD_PENDING:
            case XenObject::PATCH_APPLY_PENDING:
                SignalManager::instance().xenObjectSignal(*object).connect(sigc::mem_fun(*this, &PatchListView::onNotify));
                break;
            case XenObject::PATCH_DOWNLOADED:
            case XenObject::PATCH_DOWNLOAD_FAILED:
            case XenObject::PATCH_UPLOADED:
            case XenObject::PATCH_UPLOAD_FAILED:
            case XenObject::PATCH_APPLIED:
            case XenObject::PATCH_APPLY_FAILED:
            {
                Glib::ustring msg0 = Glib::ustring::compose("%1: ", patchRecord->label);
                Glib::ustring msg1 = patch->getOutput();
                Glib::ustring msg2 = patch->getErrorOutput();
                if (msg2.empty())
                {
                    if (!msg1.empty())
                    {
                        View::instance().showInfo(msg0 + msg1);
                    }
                }
                else if (msg1.empty())
                {
                    View::instance().showWarning(msg0 + msg2);
                }
                else
                {
                    View::instance().showWarning(msg0 + msg1 + "\n" + msg2);
                }
                break;
            }
            default:
                break;
            }
            break;
        }
        iter++;
    }
}
