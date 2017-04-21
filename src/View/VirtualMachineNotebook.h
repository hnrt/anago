// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINENOTEBOOK_H
#define HNRT_VIRTUALMACHINENOTEBOOK_H


#include "CpuGraph.h"
#include "InputOutputGraph.h"
#include "MemoryGraph.h"
//#include "NameValueListViewSw.h"
#include "Notebook.h"
#include "SnapshotView.h"
#include "VirtualMachinePropertyView.h"


namespace hnrt
{
    class ConsoleViewImpl;
    class ConsoleViewKeyboardInputFilterImpl;
    class PerformanceMonitor;
    class VirtualMachine;

    class VirtualMachineNotebook
        : public Notebook
    {
    public:

        static RefPtr<Notebook> create(const RefPtr<VirtualMachine>&);

        virtual ~VirtualMachineNotebook();

    protected:

        VirtualMachineNotebook(const RefPtr<VirtualMachine>&);
        VirtualMachineNotebook(const VirtualMachineNotebook&);
        void operator =(const VirtualMachineNotebook&);
        void onDisableConsoleChanged();
        void onScaleConsoleChanged();
        void onScaleByThreadsChanged();
        void onSessionUpdated(RefPtr<XenObject>, int);
        void onUpdated(RefPtr<XenObject>, int);
        void update();
        void openConsole();
        void closeConsole();
        void onConsoleResized(Gtk::Allocation&);
        void onUnfullscreen();
        void onDockingContainerResized(Gtk::Allocation&);
        void onFloatingContainerResized(Gtk::Allocation&);
        void reparentConsoleViewOnFullscreen();
        bool onFloatingContainerStateChanged(GdkEventWindowState*);
        void onSeparate();
        void onFloatingWindowHidden();
        void dockConsole(bool);
        void updatePerformanceStats();
        void updateSnapshots();

        RefPtr<VirtualMachine> _vm;

        VirtualMachinePropertyView _propertyView;

        RefPtr<ConsoleViewImpl> _consoleView;

        RefPtr<ConsoleViewKeyboardInputFilterImpl> _keyboardInputFilter;

        Gtk::ScrolledWindow _conDockingSw;
        Gtk::HBox _conBox1;
        Gtk::VBox _conBox2;
        Gtk::HBox _conBox3;
        Gtk::Button _conButton;
        Gtk::Button _sasButton;

        bool _consoleDocked;
        bool _consoleFullscreen;
        int _consoleWidth;
        int _consoleHeight;

        Gtk::Window _conFloatingWin;
        Gtk::ScrolledWindow _conFloatingSw;
        Gtk::HBox _conFloatingBox1;
        Gtk::VBox _conFloatingBox2;

        Gtk::VBox _ssvBox;
        SnapshotView _ssv;

        Gtk::VBox _pfmBox;
        Gtk::ScrolledWindow _pfmSw;
        Gtk::VBox _pfmBox2;
        Gtk::Label _cpuLabel;
        CpuGraph _cpuGraph;
        Gtk::Label _memLabel;
        MemoryGraph _memGraph;
        Gtk::Label _netLabel;
        InputOutputGraph _netGraph;
        Gtk::Label _dskLabel;
        InputOutputGraph _dskGraph;
        sigc::connection _pfmUpdated;

        Gtk::VBox _optBox;
        Gtk::CheckButton _disableConsole;
        Gtk::CheckButton _scaleConsole;
        Gtk::CheckButton _scaleByThreads;

        xen_vm_power_state _lastPowerState;

        sigc::connection _connectionSession;
        sigc::connection _connection;

        bool _updating;
    };
}


#endif //!HNRT_VIRTUALMACHINENOTEBOOK_H
