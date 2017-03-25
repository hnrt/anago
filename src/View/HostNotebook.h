// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_HOSTNOTEBOOK_H
#define HNRT_HOSTNOTEBOOK_H


#include "Base/RefPtr.h"
#include "CpuGraph.h"
#include "HostMenu.h"
#include "InputOutputGraph.h"
#include "MemoryGraph.h"
#include "Notebook.h"
#include "NameValueListViewSw.h"
#include "PatchListViewSw.h"


namespace hnrt
{
    class Host;
    class XenObject;

    class HostNotebook
        : public Notebook
    {
    public:

        static RefPtr<Notebook> create(const RefPtr<Host>&);

        virtual ~HostNotebook();

    protected:

        HostNotebook(const RefPtr<Host>&);
        HostNotebook(const HostNotebook&);
        void operator =(const HostNotebook&);
        void onAutoConnectChanged();
        void onSessionUpdated(RefPtr<XenObject>, int);
        void onHostUpdated(RefPtr<XenObject>, int);
        void update();
        void updatePerformaceStats();

        Gtk::VBox _genBox;
        NameValueListViewSw _genLvSw;
        NameValueListView& _genLv;
        HostMenu _genLvMenu;

        Gtk::VBox _cpuBox;
        NameValueListViewSw _cpuLvSw;
        NameValueListView& _cpuLv;

        Gtk::VBox _memBox;
        NameValueListViewSw _memLvSw;
        NameValueListView& _memLv;

        Gtk::VBox _swvBox;
        NameValueListViewSw _swvLvSw;
        NameValueListView& _swvLv;

        Gtk::VBox _patBox;
        PatchListViewSw _patLvSw;
        PatchListView& _patLv;

        Gtk::VBox _pfmBox;
        Gtk::ScrolledWindow _pfmSw;
        Gtk::VBox _pfmBox2;
        Gtk::Label _cpuLabel;
        CpuGraph _cpuGraph;
        Gtk::Label _memLabel;
        MemoryGraph _memGraph;
        Gtk::Label _netLabel;
        InputOutputGraph _netGraph;

        Gtk::VBox _optBox;
        Gtk::CheckButton _autoConnect;

        RefPtr<Host> _host;

        sigc::connection _connectionSession;
        sigc::connection _connectionHost;
    };
}


#endif //!HNRT_HOSTNOTEBOOK_H
