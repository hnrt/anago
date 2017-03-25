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

        typedef sigc::signal<void, RefPtr<XenObject> > SignalNodeCreated;

        HostTreeView();
        virtual ~HostTreeView();
        void clear();
        SignalNodeCreated signalNodeCreated() { return _signalNodeCreated; }
        Glib::SignalProxy0<void> signalSelectionChanged() { return get_selection()->signal_changed(); }
        Gtk::TreeIter getFirst() const;
        bool isSelected(const Gtk::TreeIter& iter) const { return get_selection()->is_selected(iter); }
        RefPtr<XenObject> getObject(const Gtk::TreeIter&) const;
        void updateDisplayOrder();

    protected:

        typedef bool (HostTreeView::*AddObject)(RefPtr<XenObject>);
        typedef void (HostTreeView::*UpdateObject)(RefPtr<XenObject>, int);

        HostTreeView(const HostTreeView&);
        void operator =(const HostTreeView&);
        void onObjectCreated(RefPtr<XenObject>, int);
        void onObjectUpdated(RefPtr<XenObject>, int);
        AddObject getAdd(const RefPtr<XenObject>&);
        UpdateObject getUpdate(const RefPtr<XenObject>&);
        bool addHost(RefPtr<XenObject>);
        void updateHost(RefPtr<XenObject>, int);
        bool addVm(RefPtr<XenObject>);
        void updateVm(RefPtr<XenObject>, int);
        bool addSr(RefPtr<XenObject>);
        void updateSr(RefPtr<XenObject>, int);
        bool addNw(RefPtr<XenObject>);
        void updateNw(RefPtr<XenObject>, int);
        bool addNothing(RefPtr<XenObject>);
        void updateNothing(RefPtr<XenObject>, int);
        void reorder(RefPtr<VirtualMachine>, Gtk::TreeIter&);
        void reorder(RefPtr<StorageRepository>, Gtk::TreeIter&);
        virtual bool on_button_press_event(GdkEventButton*);
        void remove(const RefPtr<XenObject>&);
        bool remove(const RefPtr<XenObject>&, Gtk::TreeIter);
        Gtk::TreeModel::Row findHost(const RefPtr<XenObject>&, Gtk::TreeIter&, bool = false);
        Gtk::TreeModel::Row findVm(const RefPtr<XenObject>&, Gtk::TreeIter&, bool = false);
        Gtk::TreeModel::Row findSr(const RefPtr<XenObject>&, Gtk::TreeIter&, bool = false);
        Gtk::TreeModel::Row findNw(const RefPtr<XenObject>&, Gtk::TreeIter&, bool = false);

        Glib::RefPtr<HostTreeStore> _store;
        HostMenu _menuHost;
        //VirtualMachineMenu _menuVm;
        //StorageRepositoryMenu _menuSr;

        SignalNodeCreated _signalNodeCreated;
    };
}


#endif //!HNRT_HOSTTREEVIEW_H
