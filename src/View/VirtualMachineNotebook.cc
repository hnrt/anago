// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <string.h>
#include "Base/StringBuffer.h"
#include "Net/Console.h"
#include "Controller/Controller.h"
#include "Controller/SignalManager.h"
#include "Exception/Exception.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "XenServer/Host.h"
#include "XenServer/PerformanceMonitor.h"
#include "XenServer/Session.h"
#include "XenServer/VirtualBlockDevice.h"
#include "XenServer/VirtualDiskImage.h"
#include "XenServer/VirtualInterface.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObjectStore.h"
#include "ConsoleViewImpl.h"
#include "ConsoleViewKeyboardInputFilterImpl.h"
#include "PixStore.h"
#include "VirtualMachineDevicePage.h"
#include "VirtualMachineNotebook.h"


using namespace hnrt;


RefPtr<Notebook> VirtualMachineNotebook::create(const RefPtr<VirtualMachine>& vm)
{
    return RefPtr<Notebook>(new VirtualMachineNotebook(vm));
}


VirtualMachineNotebook::VirtualMachineNotebook(const RefPtr<VirtualMachine>& vm)
    : _vm(vm)
    , _propertyView(vm)
    , _consoleView(ConsoleViewImpl::create())
    , _consoleDocked(true)
    , _consoleFullscreen(false)
    , _lastPowerState(XEN_VM_POWER_STATE_UNDEFINED)
    , _updating(false)
{
    TRACEFUN(this, "VirtualMachineNotebook::ctor(%s)", vm->getName().c_str());

    append_page(_propertyView, Glib::ustring(gettext("Properties")));

    _keyboardInputFilter = ConsoleViewKeyboardInputFilterImpl::create();
    _keyboardInputFilter->signalUnfullscreen().connect(sigc::mem_fun(*this, &VirtualMachineNotebook::onUnfullscreen));

    _consoleView->signal_size_allocate().connect(sigc::mem_fun(*this, &VirtualMachineNotebook::onConsoleResized));
    _consoleView->setKeyboardInputFilter(RefPtr<ConsoleViewKeyboardInputFilter>::castStatic(_keyboardInputFilter));

    _conFloatingBox2.set_spacing(0);
    _conFloatingBox2.set_border_width(0);

    _conFloatingBox1.set_spacing(0);
    _conFloatingBox1.set_border_width(0);
    _conFloatingBox1.pack_start(_conFloatingBox2, Gtk::PACK_EXPAND_PADDING);

    _conFloatingSw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _conFloatingSw.set_shadow_type(Gtk::SHADOW_NONE);
    _conFloatingSw.set_border_width(0);
    _conFloatingSw.signal_size_allocate().connect(sigc::mem_fun(*this, &VirtualMachineNotebook::onFloatingContainerResized));
    _conFloatingSw.add(_conFloatingBox1);

    _conFloatingWin.set_icon(PixStore::instance().getApp());
    _conFloatingWin.add(_conFloatingSw);
    _conFloatingWin.signal_hide().connect(sigc::mem_fun(*this, &VirtualMachineNotebook::onFloatingWindowHidden));
    _conFloatingWin.signal_window_state_event().connect(sigc::mem_fun(*this, &VirtualMachineNotebook::onFloatingContainerStateChanged));

    _conButton.set_label(gettext("Undock"));
    _conButton.set_tooltip_text(gettext("Detach the console from the main window and float it as another window"));
    _conButton.signal_clicked().connect(sigc::mem_fun(*this, &VirtualMachineNotebook::onSeparate));
    _sasButton.set_label(gettext("Ctrl+Alt+Del"));
    _sasButton.set_tooltip_text(gettext("Send Ctrl+Alt+Del to the virtual machine"));
    _sasButton.signal_clicked().connect(sigc::mem_fun(Controller::instance(), &Controller::sendCtrlAltDelete));

    _conBox3.set_spacing(0);
    _conBox3.pack_start(_conButton, Gtk::PACK_SHRINK);
    _conBox3.pack_start(_sasButton, Gtk::PACK_SHRINK);

    _conBox2.set_spacing(0);
    _conBox2.pack_start(*_consoleView, Gtk::PACK_EXPAND_PADDING);
    _conBox2.pack_start(_conBox3, Gtk::PACK_SHRINK, 8);

    _conBox1.set_spacing(0);
    _conBox1.pack_start(_conBox2, Gtk::PACK_EXPAND_PADDING);

    _conDockingSw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _conDockingSw.signal_size_allocate().connect(sigc::mem_fun(*this, &VirtualMachineNotebook::onDockingContainerResized));
    _conDockingSw.add(_conBox1);

    append_page(_conDockingSw, Glib::ustring(gettext("Console")));

    _consoleView->enableScale(Model::instance().getConsoleScale(_vm->getUUID()));

    _ssv.getTreeView().set(vm);
    _ssvBox.pack_start(_ssv);
    append_page(_ssvBox, Glib::ustring(gettext("Snapshots")));

    _pfmSw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _pfmSw.add(_pfmBox2);
    _pfmBox.pack_start(_pfmSw);
    _cpuLabel.set_label(gettext("CPU Load History"));
    _cpuLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _pfmBox2.pack_start(_cpuLabel, Gtk::PACK_SHRINK);
    _pfmBox2.pack_start(_cpuGraph);
    _memLabel.set_label(gettext("Memory Usage History"));
    _memLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _pfmBox2.pack_start(_memLabel, Gtk::PACK_SHRINK);
    _pfmBox2.pack_start(_memGraph);
    _netLabel.set_label(gettext("Network I/O History"));
    _netLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _pfmBox2.pack_start(_netLabel, Gtk::PACK_SHRINK);
    _pfmBox2.pack_start(_netGraph);
    _dskLabel.set_label(gettext("Disk I/O History"));
    _dskLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _pfmBox2.pack_start(_dskLabel, Gtk::PACK_SHRINK);
    _pfmBox2.pack_start(_dskGraph);
    append_page(_pfmBox, Glib::ustring(gettext("Performance")));

    _disableConsole.set_label(gettext("Disable console"));
    _disableConsole.set_active(!Model::instance().getConsoleEnabled(_vm->getUUID()));
    _disableConsole.signal_toggled().connect(sigc::mem_fun(*this, &VirtualMachineNotebook::onDisableConsoleChanged));
    _optBox.pack_start(_disableConsole, Gtk::PACK_SHRINK);

    _scaleConsole.set_label(gettext("Scale console window"));
    _scaleConsole.set_active(Model::instance().getConsoleScale(_vm->getUUID()));
    _scaleConsole.signal_toggled().connect(sigc::mem_fun(*this, &VirtualMachineNotebook::onScaleConsoleChanged));
    _optBox.pack_start(_scaleConsole, Gtk::PACK_SHRINK);

    _scaleByThreads.set_label(gettext("Scale console window by multi-threads"));
    _scaleByThreads.set_active(true);
    _scaleByThreads.signal_toggled().connect(sigc::mem_fun(*this, &VirtualMachineNotebook::onScaleByThreadsChanged));
#ifdef _DEBUG_CONSOLEVIEW
    _optBox.pack_start(_scaleByThreads, Gtk::PACK_SHRINK);
#endif

    append_page(_optBox, Glib::ustring(gettext("Options")));

    show_all_children();

    _propertyView.init();

    Model::instance().addConsole(_vm->getUUID(), _consoleView->getConsole());

    set_current_page(0);

    update();

    SignalManager& sm = SignalManager::instance();
    _connectionSession = sm.xenObjectSignal(_vm->getSession()).connect(sigc::mem_fun(*this, &VirtualMachineNotebook::onSessionUpdated));
    _connection = sm.xenObjectSignal(*_vm).connect(sigc::mem_fun(*this, &VirtualMachineNotebook::onUpdated));
}


VirtualMachineNotebook::~VirtualMachineNotebook()
{
    TRACEFUN(this, "VirtualMachineNotebook::dtor");

    closeConsole();

    _keyboardInputFilter.reset();
    _consoleView->setKeyboardInputFilter(RefPtr<ConsoleViewKeyboardInputFilter>::castStatic(_keyboardInputFilter));

    Model::instance().removeConsole(_vm->getUUID());

    _connectionSession.disconnect();
    _connection.disconnect();
}


void VirtualMachineNotebook::onDisableConsoleChanged()
{
    bool value = _disableConsole.get_active();
    Model::instance().setConsoleEnabled(_vm->getUUID(), !value);
    if (value)
    {
        closeConsole();
    }
    else if (_vm->getRecord()->power_state == XEN_VM_POWER_STATE_RUNNING)
    {
        openConsole();
    }
}


void VirtualMachineNotebook::onScaleConsoleChanged()
{
    bool scaleValue = _scaleConsole.get_active();
    Model::instance().setConsoleScale(_vm->getUUID(), scaleValue);
    _consoleView->enableScale(scaleValue);
    if (_consoleDocked)
    {
        _consoleView->onContainerResized(_conDockingSw);
    }
    else if (_consoleFullscreen)
    {
        int x0, y0, cx0, cy0, depth;
        _conFloatingWin.get_root_window()->get_geometry(x0, y0, cx0, cy0, depth);
        if (cx0 == _consoleWidth && cy0 == _consoleHeight)
        {
            _consoleView->onContainerResized(cx0, cy0);
        }
        else
        {
            _consoleView->onContainerResized(_conFloatingSw);
        }
    }
    else
    {
        _consoleView->onContainerResized(_conFloatingSw);
    }
    _consoleView->queue_draw();
}


void VirtualMachineNotebook::onScaleByThreadsChanged()
{
    bool value = _scaleByThreads.get_active();
    _consoleView->enableScaleByThreads(value);
}


void VirtualMachineNotebook::onSessionUpdated(RefPtr<XenObject> object, int what)
{
    switch (what)
    {
    case XenObject::PERFORMANCE_STATS_UPDATED:
        updatePerformanceStats();
        break;
    default:
        break;
    }
}


void VirtualMachineNotebook::onUpdated(RefPtr<XenObject> object, int what)
{
    switch (what)
    {
    case XenObject::RECORD_UPDATED:
    case XenObject::POWER_STATE_UPDATED:
    case XenObject::NAME_UPDATED:
    case XenObject::STATUS_UPDATED:
    case XenObject::SESSION_UPDATED:
        update();
        break;
    case XenObject::SNAPSHOT_UPDATED:
        updateSnapshots();
        break;
    default:
        break;
    }
}


void VirtualMachineNotebook::update()
{
    if (_updating)
    {
        return;
    }

    _updating = true;

    XenPtr<xen_vm_record> record = _vm->getRecord();

    if (_lastPowerState != record->power_state)
    {
        // change can be caught here.
        _lastPowerState = record->power_state;
    }

    _conFloatingWin.set_title(record->name_label && *record->name_label ? record->name_label : "Anago");

    if (record->power_state == XEN_VM_POWER_STATE_RUNNING &&
        Model::instance().getConsoleEnabled(_vm->getUUID()))
    {
        openConsole();
    }

    _ssv.getTreeView().set(_vm);

    _updating = false;
}


void VirtualMachineNotebook::openConsole()
{
    if (!_consoleView->getConsole()->isActive())
    {
        _consoleWidth = 0;
        _consoleHeight = 0;
        Glib::ustring location;
        Glib::ustring authString;
        {
            Session& session = _vm->getSession();
            Session::Lock lock(session);
            location = _vm->getConsoleLocation();
            authString = session.getConnectSpec().getBasicAuthString();
        }
        _consoleView->show();
        _consoleView->open(location.c_str(), authString.c_str());
    }
}


void VirtualMachineNotebook::closeConsole()
{
    _consoleView->close();
}


void VirtualMachineNotebook::onConsoleResized(Gtk::Allocation&)
{
    if (_consoleFullscreen)
    {
        reparentConsoleViewOnFullscreen();
    }
}


void VirtualMachineNotebook::onUnfullscreen()
{
    if (_consoleFullscreen)
    {
        _consoleFullscreen = false;
        _conFloatingWin.unfullscreen();
        _consoleView->get_parent()->remove(*_consoleView);
        _conFloatingWin.remove();
        _conFloatingBox2.pack_start(*_consoleView, Gtk::PACK_EXPAND_PADDING);
        _conFloatingWin.add(_conFloatingSw);
        _consoleView->onContainerResized(_conFloatingSw);
        _consoleWidth = 0;
        _consoleHeight = 0;
    }
}


void VirtualMachineNotebook::onDockingContainerResized(Gtk::Allocation& unused)
{
    if (_consoleDocked)
    {
        _consoleView->onContainerResized(_conDockingSw);
    }
}


void VirtualMachineNotebook::onFloatingContainerResized(Gtk::Allocation& unused)
{
    if (!_consoleDocked)
    {
        _consoleView->onContainerResized(_conFloatingSw);
    }
}


void VirtualMachineNotebook::reparentConsoleViewOnFullscreen()
{
    int x0, y0, cx0, cy0, depth;
    _conFloatingWin.get_root_window()->get_geometry(x0, y0, cx0, cy0, depth);
    int cx = _consoleView->getFrameWidth();
    int cy = _consoleView->getFrameHeight();
    if (cx != _consoleWidth || cy != _consoleHeight)
    {
        _consoleWidth = cx;
        _consoleHeight = cy;
        _consoleView->get_parent()->remove(*_consoleView);
        _conFloatingWin.remove();
        if (cx == cx0 && cy == cy0)
        {
            _conFloatingWin.add(*_consoleView);
            _consoleView->onContainerResized(cx0, cy0);
        }
        else
        {
            _conFloatingBox2.pack_start(*_consoleView, Gtk::PACK_EXPAND_PADDING);
            _conFloatingWin.add(_conFloatingSw);
            _consoleView->onContainerResized(_conFloatingSw);
        }
    }
}


Glib::ustring GetWindowStateText(GdkWindowState state)
{
    StringBuffer s;
    int v = (int)state;
#define CHECK(x) if((v&GDK_WINDOW_STATE_##x)){s+="|"#x;v&=~GDK_WINDOW_STATE_##x;}else
    CHECK(WITHDRAWN);
    CHECK(ICONIFIED);
    CHECK(MAXIMIZED);
    CHECK(STICKY);
    CHECK(FULLSCREEN);
    CHECK(ABOVE);
    CHECK(BELOW);
#undef  CHECK
    if (v || !s.len()) s.appendFormat("|%xh", v);
    return Glib::ustring(s.str() + 1);
}


bool VirtualMachineNotebook::onFloatingContainerStateChanged(GdkEventWindowState* p)
{
    if ((p->changed_mask & GDK_WINDOW_STATE_MAXIMIZED) && (p->new_window_state & GDK_WINDOW_STATE_MAXIMIZED))
    {
        _conFloatingWin.unmaximize();
        if (!_consoleFullscreen)
        {
            _conFloatingWin.fullscreen();
        }
    }
    else if ((p->changed_mask & GDK_WINDOW_STATE_FULLSCREEN) && (p->new_window_state & GDK_WINDOW_STATE_FULLSCREEN))
    {
        if (!_consoleFullscreen)
        {
            _consoleFullscreen = true;
            reparentConsoleViewOnFullscreen();
        }
    }
    return true;
}


void VirtualMachineNotebook::onSeparate()
{
    if (_consoleDocked)
    {
        dockConsole(false);
    }
    else
    {
        dockConsole(true);
    }
}


void VirtualMachineNotebook::onFloatingWindowHidden()
{
    dockConsole(true);
}


void VirtualMachineNotebook::dockConsole(bool dock)
{
    if (_consoleDocked == dock)
    {
        return;
    }
    _consoleDocked = dock;
    if (_consoleDocked)
    {
        _conFloatingWin.deiconify();
        _conFloatingWin.hide();
        _conFloatingWin.unfullscreen();
        _conFloatingWin.unmaximize();
        _consoleFullscreen = false;
        _consoleView->get_parent()->remove(*_consoleView);
        _conBox2.remove(_conBox3);
        _conBox2.pack_start(*_consoleView, Gtk::PACK_EXPAND_PADDING);
        _conBox2.pack_start(_conBox3, Gtk::PACK_SHRINK, 8);
        _conButton.set_label(gettext("Undock"));
        _conButton.set_tooltip_text(gettext("Detach the console from the main window and float it as another window"));
        _consoleView->onContainerResized(_conDockingSw);
    }
    else
    {
        if (!_conFloatingWin.is_realized())
        {
            int x0, y0, cx0, cy0, depth;
            _conFloatingWin.get_root_window()->get_geometry(x0, y0, cx0, cy0, depth);
            int cxMax = cx0 * 8 / 10;
            int cyMax = cy0 * 8 / 10;
            int cx = _consoleView->get_width() + 2;
            int cy = _consoleView->get_height() + 2;
            if (cx > cxMax)
            {
                cx = cxMax;
            }
            if (cy > cyMax)
            {
                cy = cyMax;
            }
            _conFloatingWin.set_default_size(cx, cy);
        }
        _consoleView->get_parent()->remove(*_consoleView);
        _conFloatingBox2.pack_start(*_consoleView, Gtk::PACK_EXPAND_PADDING);
        _conFloatingWin.remove();
        _conFloatingWin.add(_conFloatingSw);
        _conFloatingWin.show_all();
        _conBox2.remove(_conBox3);
        _conBox2.pack_start(_conBox3, Gtk::PACK_EXPAND_PADDING);
        _conButton.set_label(gettext("Dock"));
        _conButton.set_tooltip_text(gettext("Attach the console back to the main window from the floating window"));
        _consoleView->onContainerResized(_conFloatingSw);
    }
}


void VirtualMachineNotebook::updatePerformanceStats()
{
    RefPtr<PerformanceMonitor> pm = _vm->getSession().getStore().getPerformanceMonitor();

    for (;;)
    {
        PerformanceMonitor::ListEntry entry = pm->getEntry(_vm->getUUID());
        if (!entry.first)
        {
            break;
        }
        StringBuffer key;
        PerformanceMonitor::Map::const_iterator i;

        _cpuGraph.addTime(entry.first);
        for (int j = 0; j < CpuGraph::kMaxElementCount; j++)
        {
            key.format("cpu%u", j);
            i = entry.second->find(key.str());
            if (i != entry.second->end())
            {
                if (i->second != "NaN")
                {
                    double v = strtod(i->second.c_str(), NULL);
                    _cpuGraph.addValue(j, round(v * 100.0));
                }
                else
                {
                    _cpuGraph.addValue(j, 0);
                }
            }
            else
            {
                break;
            }
        }
        _cpuGraph.update();

        _memGraph.addTime(entry.first);
        double memory;
        key.format("memory");
        i = entry.second->find(key.str());
        if (i != entry.second->end())
        {
            if (i->second != "NaN")
            {
                memory = strtod(i->second.c_str(), NULL);
            }
            else
            {
                memory = 0.0;
            }
        }
        else
        {
            memory = 0.0;
        }
        double memory_internal_free;
        key.format("memory_internal_free");
        i = entry.second->find(key.str());
        if (i != entry.second->end())
        {
            if (i->second != "NaN")
            {
                memory_internal_free = strtod(i->second.c_str(), NULL);
                memory_internal_free *= 1024.0;
            }
            else
            {
                memory_internal_free = 0.0;
            }
        }
        else
        {
            memory_internal_free = 0.0;
        }
#if 0
        double memory_target;
        key.format("memory_target");
        i = entry.second->find(key.str());
        if (i != entry.second->end())
        {
            if (i->second != "NaN")
            {
                memory_target = strtod(i->second.c_str(), NULL);
            }
            else
            {
                memory_target = 0.0;
            }
        }
        else
        {
            memory_target = 0.0;
        }
        (void)memory_target;
#endif
        _memGraph.addValue(0, memory / (1024.0 * 1024.0));
        _memGraph.addValue(1, (memory - memory_internal_free) / (1024.0 * 1024.0));
        _memGraph.update();

        _netGraph.addTime(entry.first);
        for (int j = 0; j < 10; j++)
        {
            double rx, tx;

            key.format("vif_%u_rx", j);
            i = entry.second->find(key.str());
            if (i != entry.second->end())
            {
                if (i->second != "NaN")
                {
                    rx = strtod(i->second.c_str(), NULL);
                }
                else
                {
                    rx = 0.0;
                }
            }
            else
            {
                continue;
            }
            
            key.format("vif_%u_tx", j);
            i = entry.second->find(key.str());
            if (i != entry.second->end())
            {
                if (i->second != "NaN")
                {
                    tx = strtod(i->second.c_str(), NULL);
                }
                else
                {
                    tx = 0.0;
                }
            }
            else
            {
                continue;
            }

            StringBuffer rxLabel, txLabel;
            rxLabel.format(gettext("Network %d receive"), j);
            txLabel.format(gettext("Network %d send"), j);
            _netGraph.addValue(j, rx, tx, rxLabel, txLabel);
        }
        _netGraph.update();

        _dskGraph.addTime(entry.first);
        for (int j = 0; j < 10; j++)
        {
            double rx, wx;

            key.format("vbd_xvd%c_read", 'a' + j);
            i = entry.second->find(key.str());
            if (i != entry.second->end())
            {
                if (i->second != "NaN")
                {
                    rx = strtod(i->second.c_str(), NULL);
                }
                else
                {
                    rx = 0.0;
                }
            }
            else
            {
                continue;
            }
            
            key.format("vbd_xvd%c_write", 'a' + j);
            i = entry.second->find(key.str());
            if (i != entry.second->end())
            {
                if (i->second != "NaN")
                {
                    wx = strtod(i->second.c_str(), NULL);
                }
                else
                {
                    wx = 0.0;
                }
            }
            else
            {
                continue;
            }

            StringBuffer rxLabel, wxLabel;
            rxLabel.format(gettext("Disk %d read"), j);
            wxLabel.format(gettext("Disk %d write"), j);
            _dskGraph.addValue(j, rx, wx, rxLabel, wxLabel);
        }
        _dskGraph.update();

        delete entry.second;
    }
}


void VirtualMachineNotebook::updateSnapshots()
{
    _ssv.getTreeView().set(_vm);
}
