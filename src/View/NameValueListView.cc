// Copyright (C) 2012-2018 Hideaki Narita


#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "NameValueListView.h"


using namespace hnrt;


NameValueListView::NameValueListView()
    : _pMenu(NULL)
{
    _store = Gtk::ListStore::create(_record);
    set_model(_store);
    append_column(gettext("Name"), _record.colName);
    append_column(gettext("Value"), _record.colValue);
    get_column(0)->set_resizable(true);
    get_column(0)->set_reorderable(false);
    get_column(1)->set_resizable(false);
    get_column(1)->set_reorderable(false);
    set_rules_hint(true);
    Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
    selection->set_mode(Gtk::SELECTION_SINGLE);
}


void NameValueListView::clear()
{
    _store->clear();
}


void NameValueListView::set(const char* name, const Glib::ustring& value, bool end)
{
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        if (row[_record.colName] == name)
        {
            row[_record.colValue] = value;
            if (end)
            {
                iter++;
                while (iter)
                {
                    iter = _store->erase(iter);
                }
            }
            return;
        }
        iter++;
    }
    Gtk::TreeModel::Row row = *_store->append();
    row[_record.colName] = Glib::ustring(name);
    row[_record.colValue] = value;
}


void NameValueListView::set(const char* name, const char* value, bool end)
{
    set(name, Glib::ustring(value ? value : ""), end);
}


void NameValueListView::set(const char* name, int64_t value, bool end)
{
    char tmp[64];
    sprintf(tmp, "%'ld", value);
    set(name, tmp, end);
}


void NameValueListView::set(const char* name, double value, bool end)
{
    char tmp[64];
    sprintf(tmp, "%'lg", value);
    set(name, tmp, end);
}


void NameValueListView::set(const char* name, bool value, bool end)
{
    set(name, value ? "true" : "false", end);
}


// time_t needs to be passed as a pointer, instead of the immediate value.
// Otherwise, the compiler will complain about not being able to overload this method because of the one with int64_t.
// i.e., time_t is equivalent to int64_t.
void NameValueListView::set(const char* name, time_t *value, bool end)
{
    struct tm x = {0};
    localtime_r(value, &x);
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "%04d-%02d-%02d %02d:%02d:%02d",
             x.tm_year + 1900, x.tm_mon + 1, x.tm_mday,
             x.tm_hour, x.tm_min, x.tm_sec);
    set(name, tmp, end);
}


void NameValueListView::set(const char* name, xen_string_string_map *value, bool end)
{
    xen_string_string_map_contents* r = value ? value->contents : NULL;
    xen_string_string_map_contents* s = value ? value->contents + value->size : NULL;
    Glib::ustring prefix = Glib::ustring::compose("%1: ", name);
    size_t prefixLen = prefix.bytes();
    Gtk::TreeModel::Row row;
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (1)
    {
        if (!iter)
        {
            while (r < s)
            {
                row = *_store->append();
                row[_record.colName] = Glib::ustring::compose("%1%2", prefix, r->key);
                row[_record.colValue] = Glib::ustring(r->val);
                r++;
            }
            return;
        }
        row = *iter;
        Glib::ustring name2 = row[_record.colName];
        if (!strncmp(name2.c_str(), prefix.c_str(), prefixLen))
        {
            break;
        }
        iter++;
    }
    while (r < s)
    {
        row[_record.colName] = Glib::ustring::compose("%1%2", prefix, r->key);
        row[_record.colValue] = Glib::ustring(r->val);
        r++;
        iter++;
        if (!iter)
        {
            while (r < s)
            {
                row = *_store->append();
                row[_record.colName] = Glib::ustring::compose("%1%2", prefix, r->key);
                row[_record.colValue] = Glib::ustring(r->val);
                r++;
            }
            return;
        }
        row = *iter;
        Glib::ustring name2 = row[_record.colName];
        if (strncmp(name2.c_str(), prefix.c_str(), prefixLen))
        {
            while (r < s)
            {
                row = *_store->insert(iter);
                row[_record.colName] = Glib::ustring::compose("%1%2", prefix, r->key);
                row[_record.colValue] = Glib::ustring(r->val);
                r++;
            }
            return;
        }
    }
    iter = _store->erase(iter);
    while (iter)
    {
        row = *iter;
        Glib::ustring name2 = row[_record.colName];
        if (strncmp(name2.c_str(), prefix.c_str(), prefixLen))
        {
            if (end)
            {
                do
                {
                    iter = _store->erase(iter);
                }
                while (iter);
            }
            break;
        }
        iter = _store->erase(iter);
    }
}


static Glib::ustring format(const char* name, size_t i)
{
    char tmp[32];
    sprintf(tmp, "[%zu]", i);
    return Glib::ustring::compose("%1%2", name, tmp);
}


void NameValueListView::set(const char* name, xen_string_set *value, bool end)
{
    char** r = value ? value->contents : NULL;
    char** s = value ? value->contents + value->size : NULL;
    Glib::ustring prefix = Glib::ustring::compose("%1[", name);
    size_t prefixLen = prefix.bytes();
    Gtk::TreeModel::Row row;
    Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
    while (1)
    {
        if (!iter)
        {
            while (r < s)
            {
                row = *_store->append();
                row[_record.colName] = format(name, r - value->contents);
                row[_record.colValue] = Glib::ustring(*r);
                r++;
            }
            return;
        }
        row = *iter;
        Glib::ustring name2 = row[_record.colName];
        if (!strncmp(name2.c_str(), prefix.c_str(), prefixLen))
        {
            break;
        }
        iter++;
    }
    while (r < s)
    {
        row[_record.colName] = format(name, r - value->contents);
        row[_record.colValue] = Glib::ustring(*r);
        r++;
        iter++;
        if (!iter)
        {
            while (r < s)
            {
                row = *_store->append();
                row[_record.colName] = format(name, r - value->contents);
                row[_record.colValue] = Glib::ustring(*r);
                r++;
            }
            return;
        }
        row = *iter;
        Glib::ustring name2 = row[_record.colName];
        if (strncmp(name2.c_str(), prefix.c_str(), prefixLen))
        {
            while (r < s)
            {
                row = *_store->insert(iter);
                row[_record.colName] = format(name, r - value->contents);
                row[_record.colValue] = Glib::ustring(*r);
                r++;
            }
            return;
        }
    }
    iter = _store->erase(iter);
    while (iter)
    {
        Glib::ustring name2 = row[_record.colName];
        if (strncmp(name2.c_str(), prefix.c_str(), prefixLen))
        {
            if (end)
            {
                do
                {
                    iter = _store->erase(iter);
                }
                while (iter);
            }
            break;
        }
        iter = _store->erase(iter);
    }
}


bool NameValueListView::on_button_press_event(GdkEventButton* event)
{
    bool retval = Gtk::TreeView::on_button_press_event(event);

    if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
    {
        Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
        Gtk::TreeIter iter = selection->get_selected();
        if (iter)
        {
            Gtk::TreeModel::Row row = *iter;
            Glib::ustring name = row[_record.colName];
            if (_pMenu)
            {
                _pMenu->popup(event->button, event->time, name.c_str());
                // The event has been handled.
                return true;
            }
        }
    }

    return retval;
}


Gtk::ScrolledWindow* NameValueListView::createScrolledWindow()
{
    Gtk::ScrolledWindow* pW = new Gtk::ScrolledWindow();
    pW->add(*this);
    pW->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    pW->set_shadow_type(Gtk::SHADOW_IN);
    return pW;
}
