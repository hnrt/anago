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
    class PerformanceMonitor;

    class HostNotebook
        : public Notebook
    {
    public:

        static Glib::RefPtr<Notebook> create(const RefPtr<Host>&);

        virtual ~HostNotebook();
        virtual const Gtk::Notebook& getInstance() const { return *this; }
        virtual Gtk::Notebook& getInstance() { return *this; }
        virtual void update();
        void initPerformaceMonitor(const RefPtr<PerformanceMonitor>&);

    protected:

        HostNotebook(const RefPtr<Host>&);
        HostNotebook(const HostNotebook&);
        void operator =(const HostNotebook&);
        void onAutoConnectChanged();
        void onPerformaceStatsUpdated(RefPtr<RefObj>, int);

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
    };
}


#endif //!HNRT_HOSTNOTEBOOK_H
