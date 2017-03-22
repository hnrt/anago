// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_NAMEVALUELISTVIEW_H
#define HNRT_NAMEVALUELISTVIEW_H


#include <gtkmm.h>
#include "XenServer/Api.h"
#include "NameValueMenu.h"


namespace hnrt
{
    class NameValueListViewSw;

    class NameValueListView
        : public Gtk::TreeView
    {
    public:

        void clear();
        void set(const char* name, const Glib::ustring& value, bool end = false);
        void set(const char* name, const char* value, bool end = false);
        void set(const char* name, int64_t value, bool end = false);
        void set(const char* name, double value, bool end = false);
        void set(const char* name, bool value, bool end = false);
        void set(const char* name, time_t *value, bool end = false);
        void set(const char* name, xen_string_string_map *value, bool end = false);
        void set(const char* name, xen_string_set *value, bool end = false);
        void setMenu(NameValueMenu* pMenu) { _pMenu = pMenu; }

    protected:

        struct Record : public Gtk::TreeModel::ColumnRecord
        {
            Gtk::TreeModelColumn<Glib::ustring> colName;
            Gtk::TreeModelColumn<Glib::ustring> colValue;

            Record()
            {
                add(colName);
                add(colValue);
            }

        private:

            Record(const Record&);
            void operator =(const Record&);
        };

        NameValueListView();
        NameValueListView(const NameValueListView&);
        void operator =(const NameValueListView&);
        virtual bool on_button_press_event(GdkEventButton*);

        Record _record;
        Glib::RefPtr<Gtk::ListStore> _store;
        NameValueMenu* _pMenu;

        friend class NameValueListViewSw;
    };
}


#endif //!HNRT_NAMEVALUELISTVIEW_H
