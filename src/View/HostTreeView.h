// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_HOSTTREEVIEW_H
#define HNRT_HOSTTREEVIEW_H


#include <gtkmm.h>
#include "Base/RefPtr.h"
#include "HostMenu.h"
#include "NetworkMenu.h"
#include "StorageRepositoryMenu.h"
#include "VirtualMachineMenu.h"


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
        bool addObject(RefPtr<XenObject>);
        void updateObject(RefPtr<XenObject>, int);
        bool addNothing(RefPtr<XenObject>);
        void updateNothing(RefPtr<XenObject>, int);
        virtual bool on_button_press_event(GdkEventButton*);
        void remove(const XenObject&);
        void remove(const XenObject&, Gtk::TreeIter);
        void reorder(const XenObject&, Gtk::TreeIter);
        Gtk::TreeIter find(const XenObject&, bool);
        Gtk::TreeIter findHost(const XenObject&, bool);
        Gtk::TreeIter findChild(const XenObject&, Gtk::TreeIter, bool);
        int compare(const XenObject&, const Gtk::TreeIter&);

        Glib::RefPtr<HostTreeStore> _store;
        HostMenu _menuHost;
        VirtualMachineMenu _menuVm;
        StorageRepositoryMenu _menuSr;
        NetworkMenu _menuNetwork;

        SignalNodeCreated _signalNodeCreated;
    };
}


#endif //!HNRT_HOSTTREEVIEW_H
