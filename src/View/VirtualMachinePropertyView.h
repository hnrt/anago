// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEPROPERTYVIEW_H
#define HNRT_VIRTUALMACHINEPROPERTYVIEW_H


#include "NameValueListView.h"
#include "VirtualBlockDeviceMenu.h"
#include "VirtualDiskImageMenu.h"
#include "VirtualInterfaceMenu.h"
#include "VirtualMachineGenMenu.h"
#include "VirtualMachineMemMenu.h"


namespace hnrt
{
    class VirtualMachine;
    class VirtualMachineDevicePage;

    class VirtualMachinePropertyView
        : public Gtk::HPaned
    {
    public:

        VirtualMachinePropertyView(const RefPtr<VirtualMachine>&);
        virtual ~VirtualMachinePropertyView();
        void init();

    protected:

        enum ProtectedConstants
        {
            NO_SELECTION = -1,
            GENERAL = 0,
            MEMORY = 1,
            STORAGE = 256,
            NETWORK = 512,
            MAX_STORAGES = 64,
            MAX_NETWORKS = 16,
        };

        class LeftPane
            : public Gtk::ScrolledWindow
        {
        public:

            LeftPane(VirtualMachinePropertyView&);
            void addEntry(Glib::RefPtr<Gdk::Pixbuf> pix, int key, const Glib::ustring& name) { _tree.addEntry(pix, key, name); }
            void removeEntry(int key) { _tree.removeEntry(key); }
            Glib::SignalProxy0<void> signalSelectionChanged() { return _tree.signalSelectionChanged(); }
            void select(int key) { _tree.select(key); }
            int getSelected() { return _tree.getSelected(); }

        protected:

            class TreeView
                : public Gtk::TreeView
            {
            public:

                TreeView(VirtualMachinePropertyView&);
                void addEntry(Glib::RefPtr<Gdk::Pixbuf>, int, const Glib::ustring&);
                void removeEntry(int);
                Glib::SignalProxy0<void> signalSelectionChanged() { return get_selection()->signal_changed(); }
                void select(int);
                int getSelected();

            protected:

                struct Record
                    : public Gtk::TreeModel::ColumnRecord
                {
                    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > colPix;
                    Gtk::TreeModelColumn<int> colKey;
                    Gtk::TreeModelColumn<Glib::ustring> colName;

                    Record();

                private:

                    Record(const Record&);
                    void operator =(const Record&);
                };

                TreeView(const TreeView&);
                void operator =(const TreeView&);
                virtual bool on_button_press_event(GdkEventButton*);

                VirtualMachinePropertyView& _parent;
                Record _record;
                Glib::RefPtr<Gtk::TreeStore> _store;
            };

            LeftPane(const LeftPane&);
            void operator =(const LeftPane&);

            TreeView _tree;
        };

        VirtualMachinePropertyView(const VirtualMachinePropertyView&);
        void operator =(const VirtualMachinePropertyView&);
        void onSelectionChanged();
        void onUpdated(RefPtr<XenObject>, int);
        void update();

        LeftPane _left;
        Gtk::VBox _right;

        Gtk::Widget* _selected;

        NameValueListView _genLv;
        Gtk::ScrolledWindow& _genSw;
        VirtualMachineGenMenu _genMenu;

        NameValueListView _memLv;
        Gtk::ScrolledWindow& _memSw;
        VirtualMachineMemMenu _memMenu;

        VirtualMachineDevicePage* _storages[MAX_STORAGES];
        VirtualBlockDeviceMenu _vbdMenu;

        VirtualMachineDevicePage* _networks[MAX_NETWORKS];
        VirtualInterfaceMenu _vifMenu;

        RefPtr<VirtualMachine> _vm;

        sigc::connection _connection;
    };
}


#endif //!HNRT_VIRTUALMACHINEPROPERTYVIEW_H
