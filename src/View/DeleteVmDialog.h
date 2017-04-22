// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_DELETEVMDIALOG_H
#define HNRT_DELETEVMDIALOG_H


#include <gtkmm.h>
#include <list>


namespace hnrt
{
    class VirtualMachine;

    class DeleteVmDialog
        : public Gtk::Dialog
    {
    public:

        DeleteVmDialog(Gtk::Window&, const VirtualMachine&);
        int getDisks(std::list<Glib::ustring>&) const;

    private:

        struct Record
            : public Gtk::TreeModel::ColumnRecord
        {
            Gtk::TreeModelColumn<Glib::ustring> colREFID;
            Gtk::TreeModelColumn<Glib::ustring> colName;
            Gtk::TreeModelColumn<Glib::ustring> colSize;
            Gtk::TreeModelColumn<Glib::ustring> colSr;
            Gtk::TreeModelColumn<Glib::ustring> colSnapshot;
            Gtk::TreeModelColumn<bool> colDelete;

            Record()
            {
                add(colREFID);
                add(colName);
                add(colSize);
                add(colSr);
                add(colSnapshot);
                add(colDelete);
            }
        };

        DeleteVmDialog(const DeleteVmDialog&);
        void operator =(const DeleteVmDialog&);
        void onToggled(const Glib::ustring&);
        void initStore(const VirtualMachine&);
        void add(const char*, const char*, const char*, const char*, const char*);

        Gtk::Label _descLabel;
        Gtk::ScrolledWindow _vdiSw;
        Gtk::TreeView _vdiLv;
        Record _record;
        Glib::RefPtr<Gtk::ListStore> _store;
    };
}


#endif //!HNRT_DELETEVMDIALOG_H
