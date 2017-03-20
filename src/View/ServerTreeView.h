// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_SERVERTREEVIEW_H
#define HNRT_SERVERTREEVIEW_H


#include <gtkmm.h>
#include "Base/RefPtr.h"
//#include "ServerMenu.h"
//#include "VirtualMachineMenu.h"
//#include "StorageRepositoryMenu.h"


namespace hnrt
{
    class Host;
    class Network;
    class ServerTreeStore;
    class StorageRepository;
    class VirtualMachine;
    class XenObject;

    class ServerTreeView
        : public Gtk::TreeView
    {
    public:

        ServerTreeView();
        virtual ~ServerTreeView();
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

        ServerTreeView(const ServerTreeView&);
        void operator =(const ServerTreeView&);
        bool add(RefPtr<Host>);
        bool add(RefPtr<VirtualMachine>);
        bool add(RefPtr<StorageRepository>);
        bool add(RefPtr<Network>);
        void reorder(RefPtr<VirtualMachine>&, Gtk::TreeIter&);
        void reorder(RefPtr<StorageRepository>&, Gtk::TreeIter&);
        virtual bool on_button_press_event(GdkEventButton*);

        Glib::RefPtr<ServerTreeStore> _store;
        //ServerMenu _menuServer;
        //VirtualMachineMenu _menuVm;
        //StorageRepositoryMenu _menuSr;
    };
}


#endif //!HNRT_SERVERTREEVIEW_H
