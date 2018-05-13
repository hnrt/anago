// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_HOSTTREESTORE_H
#define HNRT_HOSTTREESTORE_H


#include <gtkmm.h>
#include "Base/RefPtr.h"


namespace hnrt
{
    class XenObject;

    class HostTreeStore
        : public Gtk::TreeStore
    {
    public:

        struct Record : public Gtk::TreeModel::ColumnRecord
        {
            Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > colPix;
            Gtk::TreeModelColumn<Glib::ustring> colKey;
            Gtk::TreeModelColumn<Glib::ustring> colVal;
            Gtk::TreeModelColumn<RefPtr<XenObject> > colXenObject;

            Record();

        private:

            Record(const Record&);
            void operator =(const Record&);
        };

        static Glib::RefPtr<HostTreeStore> create();

        Record& record() { return _record; }

    protected:

        HostTreeStore();
        HostTreeStore(const HostTreeStore&);
        void operator =(const HostTreeStore&);
        virtual bool row_draggable_vfunc(const Gtk::TreeModel::Path& path) const;
        virtual bool row_drop_possible_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selectionData) const;

        Record _record;
    };
}


#endif //!HNRT_HOSTTREESTORE_H
