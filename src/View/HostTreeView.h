// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_HOSTTREEVIEW_H
#define HNRT_HOSTTREEVIEW_H


#include <gtkmm.h>
#include "Base/RefPtr.h"
#include "HostMenu.h"
//#include "StorageRepositoryMenu.h"
//#include "VirtualMachineMenu.h"


namespace hnrt
{
    class Host;
    class HostTreeStore;
    class Network;
    class StorageRepository;
    class VirtualMachine;
    class XenObject;

    class HostTreeView
        : public Gtk::TreeView
    {
    public:

        HostTreeView();
        virtual ~HostTreeView();
        void clear();
        bool add(RefPtr<XenObject>&);
        void remove(const RefPtr<XenObject>&);
        void update(RefPtr<XenObject>&, int);
        Glib::SignalProxy0<void> signalSelectionChanged() { return get_selection()->signal_changed(); }
        Gtk::TreeIter getFirst() const;
        bool isSelected(const Gtk::TreeIter& iter) const { return get_selection()->is_selected(iter); }
        RefPtr<XenObject> getObject(const Gtk::TreeIter&) const;
        void updateDisplayOrder();

    protected:

        HostTreeView(const HostTreeView&);
        void operator =(const HostTreeView&);
        bool add(RefPtr<Host>);
        bool add(RefPtr<VirtualMachine>);
        bool add(RefPtr<StorageRepository>);
        bool add(RefPtr<Network>);
        void reorder(RefPtr<VirtualMachine>&, Gtk::TreeIter&);
        void reorder(RefPtr<StorageRepository>&, Gtk::TreeIter&);
        virtual bool on_button_press_event(GdkEventButton*);

        Glib::RefPtr<HostTreeStore> _store;
        HostMenu _menuHost;
        //VirtualMachineMenu _menuVm;
        //StorageRepositoryMenu _menuSr;
    };
}


#endif //!HNRT_HOSTTREEVIEW_H
