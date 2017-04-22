// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINETEMPLATELISTVIEW_H
#define HNRT_VIRTUALMACHINETEMPLATELISTVIEW_H


#include <gtkmm.h>


namespace hnrt
{
    class Session;

    class VirtualMachineTemplateListView
        : public Gtk::TreeView
    {
    public:

        VirtualMachineTemplateListView(const Session&);
        Glib::ustring getSelected() const;
        Gtk::ScrolledWindow* createScrolledWindow();

    private:

        struct Record
            : public Gtk::TreeModel::ColumnRecord
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

        VirtualMachineTemplateListView(const VirtualMachineTemplateListView&);
        void operator =(const VirtualMachineTemplateListView&);
        void initStore(const Session&);

        Record _record;
        Glib::RefPtr<Gtk::ListStore> _store;
    };
}


#endif //!HNRT_VIRTUALMACHINETEMPLATELISTVIEW_H
