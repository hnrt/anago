// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_NETWORKNOTEBOOK_H
#define HNRT_NETWORKNOTEBOOK_H


#include "Base/RefPtr.h"
#include "NameValueListView.h"
#include "NetworkMenu.h"
#include "Notebook.h"


namespace hnrt
{
    class Network;

    class NetworkNotebook
        : public Notebook
    {
    public:

        static RefPtr<Notebook> create(const RefPtr<Network>&);

        virtual ~NetworkNotebook();

    protected:

        NetworkNotebook(const RefPtr<Network>&);
        NetworkNotebook(const NetworkNotebook&);
        void operator =(const NetworkNotebook&);
        void onNetworkUpdated(RefPtr<XenObject>, int);

        Gtk::VPaned _genBox;

        Gtk::VBox _networkBox;
        Gtk::Label _networkLabel;
        NameValueListView _networkLv;
        NetworkMenu _networkMenu;

        Gtk::VBox _pifBox;
        Gtk::Label _pifLabel;
        NameValueListView _pifLv;

        RefPtr<Network> _network;

        sigc::connection _connection;
    };
}


#endif //!HNRT_NETWORKNOTEBOOK_H
