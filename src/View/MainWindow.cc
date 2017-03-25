// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "App/Constants.h"
#include "Controller/Controller.h"
#include "Controller/SignalManager.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "XenServer/Host.h"
#include "XenServer/PerformanceMonitor.h"
#include "XenServer/Session.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObjectStore.h"
#include "HostNotebook.h"
#include "NoContentsNotebook.h"
#include "NotebookFactory.h"
#include "MainWindow.h"
#include "PixStore.h"
#include "View.h"


using namespace hnrt;


MainWindow::MainWindow()
    : _stockIdAddHost("AddHost")
    , _stockIdPowerOn("PowerOn")
    , _stockIdPowerOff("PowerOff")
    , _stockIdAddVm("AddVm")
    , _stockIdStartVm("StartVm")
    , _stockIdShutdownVm("ShutdownVm")
    , _stockIdRebootVm("RebootVm")
    , _stockIdSuspendVm("SuspendVm")
    , _stockIdResumeVm("ResumeVm")
    , _stockIdChangeCd("ChangeCd")
    , _stockIdAuth("Auth")
    , _width(0)
    , _height(0)
    , _windowState((GdkWindowState)0)
{
    Trace trace("MainWindow::ctor");

    initStockItems();

    set_icon(PixStore::instance().getApp());

    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();

    actionGroup->add(Gtk::Action::create("File", gettext("_File")));

    actionGroup->add(Gtk::Action::create("Quit", Gtk::Stock::QUIT),
                     Gtk::AccelKey("<control><shift>q"),
                     sigc::mem_fun(Controller::instance(), &Controller::quit));
    actionGroup->add(Gtk::Action::create("AddHost", _stockIdAddHost),
                     sigc::mem_fun(Controller::instance(), &Controller::addHost));
    actionGroup->add(Gtk::Action::create("EditConnectSpec", gettext("_Edit host")),
                     sigc::mem_fun(Controller::instance(), &Controller::editHost));
    actionGroup->add(Gtk::Action::create("RemoveHost", gettext("_Remove host")),
                     sigc::mem_fun(Controller::instance(), &Controller::removeHost));

    actionGroup->add(Gtk::Action::create("Edit", gettext("_Edit")));

    actionGroup->add(Gtk::Action::create("EditHost", gettext("_Host")));

    actionGroup->add(Gtk::Action::create("Connect", Gtk::Stock::CONNECT),
                     Gtk::AccelKey("<control><shift>c"),
                     sigc::mem_fun(Controller::instance(), &Controller::connect));
    actionGroup->add(Gtk::Action::create("Disconnect", Gtk::Stock::DISCONNECT),
                     Gtk::AccelKey("<control><shift>d"),
                     sigc::mem_fun(Controller::instance(), &Controller::disconnect));
    actionGroup->add(Gtk::Action::create("ChangeHost", gettext("_Change")));
    actionGroup->add(Gtk::Action::create("ChangeHostName", gettext("_Label/Description")),
                     sigc::mem_fun(Controller::instance(), &Controller::changeHostName));
    actionGroup->add(Gtk::Action::create("TurnOnHost", _stockIdPowerOn),
                     sigc::mem_fun(Controller::instance(), &Controller::wakeHost));
    actionGroup->add(Gtk::Action::create("ShutdownHost", gettext("_Shut down")),
                     sigc::mem_fun(Controller::instance(), &Controller::shutdownHosts));
    actionGroup->add(Gtk::Action::create("RebootHost", gettext("_Reboot")),
                     sigc::mem_fun(Controller::instance(), &Controller::restartHosts));

    actionGroup->add(Gtk::Action::create("EditVM", gettext("_VM")));

    actionGroup->add(Gtk::Action::create("StartVm", _stockIdStartVm),
                     sigc::mem_fun(Controller::instance(), &Controller::startVm));
    actionGroup->add(Gtk::Action::create("ShutdownVm", _stockIdShutdownVm),
                     sigc::mem_fun(Controller::instance(), &Controller::shutdownVm));
    actionGroup->add(Gtk::Action::create("RebootVm", _stockIdRebootVm),
                     sigc::mem_fun(Controller::instance(), &Controller::rebootVm));
    actionGroup->add(Gtk::Action::create("SuspendVm", _stockIdSuspendVm),
                     sigc::mem_fun(Controller::instance(), &Controller::suspendVm));
    actionGroup->add(Gtk::Action::create("ResumeVm", _stockIdResumeVm),
                     sigc::mem_fun(Controller::instance(), &Controller::resumeVm));
    actionGroup->add(Gtk::Action::create("ChangeCd", _stockIdChangeCd),
                     sigc::mem_fun(Controller::instance(), &Controller::changeCd));
    actionGroup->add(Gtk::Action::create("SendCtrlAltDelete", _stockIdAuth),
                     sigc::mem_fun(Controller::instance(), &Controller::sendCtrlAltDelete));
    actionGroup->add(Gtk::Action::create("AddVm", _stockIdAddVm),
                     sigc::mem_fun(Controller::instance(), &Controller::addVm));
    actionGroup->add(Gtk::Action::create("CopyVM", gettext("_Copy")),
                     sigc::mem_fun(Controller::instance(), &Controller::copyVm));
    actionGroup->add(Gtk::Action::create("DeleteVM", gettext("_Delete")),
                     sigc::mem_fun(Controller::instance(), &Controller::deleteVm));
    actionGroup->add(Gtk::Action::create("SnapshotVM", gettext("S_napshot")),
                     sigc::mem_fun(Controller::instance(), &Controller::snapshotVm));
    actionGroup->add(Gtk::Action::create("ExportVM", gettext("E_xport")),
                     sigc::mem_fun(Controller::instance(), &Controller::exportVm));
    actionGroup->add(Gtk::Action::create("ImportVM", gettext("_Import")),
                     sigc::mem_fun(Controller::instance(), &Controller::importVm));
    actionGroup->add(Gtk::Action::create("VerifyVM", gettext("_Verify")),
                     sigc::mem_fun(Controller::instance(), &Controller::verifyVm));

    actionGroup->add(Gtk::Action::create("VMEmergency", gettext("_Emergency")));

    actionGroup->add(Gtk::Action::create("HardShutdownVm", _stockIdShutdownVm),
                     sigc::mem_fun(Controller::instance(), &Controller::hardShutdownVm));
    actionGroup->add(Gtk::Action::create("HardRebootVm", _stockIdRebootVm),
                     sigc::mem_fun(Controller::instance(), &Controller::hardRebootVm));

    actionGroup->add(Gtk::Action::create("VMChange", gettext("Chan_ge")));

    actionGroup->add(Gtk::Action::create("ChangeVMName", gettext("_Label/Description")),
                     sigc::mem_fun(Controller::instance(), &Controller::changeVmName));
    actionGroup->add(Gtk::Action::create("ChangeCPU", gettext("_CPU")),
                     sigc::mem_fun(Controller::instance(), &Controller::changeCpu));
    actionGroup->add(Gtk::Action::create("ChangeRAM", gettext("_Memory")),
                     sigc::mem_fun(Controller::instance(), &Controller::changeMemory));
    actionGroup->add(Gtk::Action::create("ChangeShadowMemory", gettext("_Shadow memory")),
                     sigc::mem_fun(Controller::instance(), &Controller::changeShadowMemory));
    actionGroup->add(Gtk::Action::create("ChangeVGA", gettext("_Video")),
                     sigc::mem_fun(Controller::instance(), &Controller::changeVga));
    actionGroup->add(Gtk::Action::create("AttachHdd", gettext("Attach _HDD")),
                     sigc::mem_fun(Controller::instance(), &Controller::attachHdd));
    actionGroup->add(Gtk::Action::create("AttachCd", gettext("Attach C_D drive")),
                     sigc::mem_fun(Controller::instance(), &Controller::attachCd));
    actionGroup->add(Gtk::Action::create("AttachNic", gettext("Attach _NIC")),
                     sigc::mem_fun(Controller::instance(), &Controller::attachNic));

    actionGroup->add(Gtk::Action::create("EditSR", gettext("S_R")));

    actionGroup->add(Gtk::Action::create("AddHdd", gettext("Add _HDD")),
                     sigc::mem_fun(Controller::instance(), &Controller::addHdd));
    actionGroup->add(Gtk::Action::create("AddCIFS", gettext("_Add CIFS")),
                     sigc::mem_fun(Controller::instance(), &Controller::addCifs));
    actionGroup->add(Gtk::Action::create("DeleteCifs", gettext("_Delete CIFS")),
                     sigc::mem_fun(Controller::instance(), &Controller::deleteCifs));

    actionGroup->add(Gtk::Action::create("SRChange", gettext("_Change")));

    actionGroup->add(Gtk::Action::create("ChangeSRName", gettext("_Label/Description")),
                     sigc::mem_fun(Controller::instance(), &Controller::changeSrName));
    actionGroup->add(Gtk::Action::create("SetDefaultSR", gettext("Set _default")),
                     sigc::mem_fun(Controller::instance(), &Controller::setDefaultSr));

    actionGroup->add(Gtk::Action::create("View", gettext("_View")));

    actionGroup->add(Gtk::Action::create("OpenExportWindow", gettext("Open V_M import/export window")),
                     sigc::mem_fun(Controller::instance(), &Controller::openVmStatusWindow));

    actionGroup->add(Gtk::Action::create("Help", gettext("_Help")));

    actionGroup->add(Gtk::Action::create("About", Gtk::Stock::ABOUT),
                     sigc::mem_fun(Controller::instance(), &Controller::showAbout));

    _uiManager = Gtk::UIManager::create();
    _uiManager->insert_action_group(actionGroup);
    add_accel_group(_uiManager->get_accel_group());

    Glib::ustring uiInfo =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu name='File' action='File'>"
        "      <menuitem action='AddHost'/>"
        "      <menuitem action='EditConnectSpec'/>"
        "      <menuitem action='RemoveHost'/>"
        "      <separator/>"
        "      <menuitem action='Quit'/>"
        "    </menu>"
        "    <menu name='Edit' action='Edit'>"
        "      <menu name='Host' action='EditHost'>"
        "        <menuitem name='Connect' action='Connect'/>"
        "        <menuitem name='Disconnect' action='Disconnect'/>"
        "        <separator/>"
        "        <menu name='Change' action='ChangeHost'>"
        "          <menuitem name='Name' action='ChangeHostName'/>"
        "        </menu>"
        "        <separator/>"
        "        <menuitem name='TurnOn' action='TurnOnHost'/>"
        "        <menuitem name='Shutdown' action='ShutdownHost'/>"
        "        <menuitem name='Reboot' action='RebootHost'/>"
        "      </menu>"
        "      <menu name='VM' action='EditVM'>"
        "        <menuitem name='Start' action='StartVm'/>"
        "        <menuitem name='Suspend' action='SuspendVm'/>"
        "        <menuitem name='Resume' action='ResumeVm'/>"
        "        <menuitem name='Shutdown' action='ShutdownVm'/>"
        "        <menuitem name='Reboot' action='RebootVm'/>"
        "        <menu name='Emergency' action='VMEmergency'>"
        "          <menuitem name='Shutdown' action='HardShutdownVm'/>"
        "          <menuitem name='Reboot' action='HardRebootVm'/>"
        "        </menu>"
        "        <separator/>"
        "        <menuitem name='ChangeCd' action='ChangeCd'/>"
        "        <separator/>"
        "        <menuitem name='SendCtrlAltDelete' action='SendCtrlAltDelete'/>"
        "        <separator/>"
        "        <menu name='Change' action='VMChange'>"
        "          <menuitem name='Name' action='ChangeVMName'/>"
        "          <menuitem name='CPU' action='ChangeCPU'/>"
        "          <menuitem name='Memory' action='ChangeRAM'/>"
        "          <menuitem name='ShadowMemory' action='ChangeShadowMemory'/>"
        "          <menuitem name='Video' action='ChangeVGA'/>"
        "          <menuitem name='HDD' action='AttachHdd'/>"
        "          <menuitem name='CD' action='AttachCd'/>"
        "          <menuitem name='NIC' action='AttachNic'/>"
        "        </menu>"
        "        <separator/>"
        "        <menuitem name='Add' action='AddVm'/>"
        "        <menuitem name='Copy' action='CopyVM'/>"
        "        <menuitem name='Delete' action='DeleteVM'/>"
        "        <menuitem name='Snapshot' action='SnapshotVM'/>"
        "        <menuitem name='Export' action='ExportVM'/>"
        "        <menuitem name='Import' action='ImportVM'/>"
        "        <menuitem name='Verify' action='VerifyVM'/>"
        "      </menu>"
        "      <menu name='SR' action='EditSR'>"
        "        <menu name='Change' action='SRChange'>"
        "          <menuitem name='Name' action='ChangeSRName'/>"
        "          <menuitem name='Default' action='SetDefaultSR'/>"
        "        </menu>"
        "        <separator/>"
        "        <menuitem name='AddHdd' action='AddHdd'/>"
        "        <separator/>"
        "        <menuitem name='AddCIFS' action='AddCIFS'/>"
        "        <menuitem name='DeleteCifs' action='DeleteCifs'/>"
        "      </menu>"
        "    </menu>"
        "    <menu name='View' action='View'>"
        "      <menuitem name='OpenExportWindow' action='OpenExportWindow'/>"
        "    </menu>"
        "    <menu name='Help' action='Help'>"
        "      <menuitem name='About' action='About'/>"
        "    </menu>"
        "  </menubar>"
        "  <toolbar name='ToolBar'>"
        "    <toolitem name='AddHost' action='AddHost'/>"
        "    <toolitem name='Connect' action='Connect'/>"
        "    <toolitem name='Disconnect' action='Disconnect'/>"
        "    <toolitem name='AddVm' action='AddVm'/>"
        "    <toolitem name='StartVm' action='StartVm'/>"
        "    <toolitem name='ResumeVm' action='ResumeVm'/>"
        "    <toolitem name='SuspendVm' action='SuspendVm'/>"
        "    <toolitem name='ShutdownVm' action='ShutdownVm'/>"
        "    <toolitem name='RebootVm' action='RebootVm'/>"
        "    <toolitem name='SendSAS' action='SendCtrlAltDelete'/>"
        "    <toolitem name='ChangeCd' action='ChangeCd'/>"
        "    <toolitem name='Quit' action='Quit'/>"
        "  </toolbar>"
        "</ui>";
    try
    {
        _uiManager->add_ui_from_string(uiInfo);
    }
    catch (const Glib::Error& ex)
    {
        Logger::instance().error("add_ui_from_string: %s", ex.what().c_str());
    }

    add(_box);

    Gtk::Widget* menubar = _uiManager->get_widget("/MenuBar");
    _box.pack_start(*menubar, Gtk::PACK_SHRINK);

    Gtk::Widget* toolbar = _uiManager->get_widget("/ToolBar");
    _box.pack_start(*toolbar, Gtk::PACK_SHRINK);

    _uiManager->get_widget("/ToolBar/Quit")->set_tooltip_text(gettext("Quit"));
    _uiManager->get_widget("/ToolBar/AddHost")->set_tooltip_text(gettext("Add host"));
    _uiManager->get_widget("/ToolBar/Connect")->set_tooltip_text(gettext("Connect to host"));
    _uiManager->get_widget("/ToolBar/Disconnect")->set_tooltip_text(gettext("Disconnect from host"));
    _uiManager->get_widget("/ToolBar/AddVm")->set_tooltip_text(gettext("New VM"));
    _uiManager->get_widget("/ToolBar/StartVm")->set_tooltip_text(gettext("Start VM"));
    _uiManager->get_widget("/ToolBar/ShutdownVm")->set_tooltip_text(gettext("Shutdown VM"));
    _uiManager->get_widget("/ToolBar/RebootVm")->set_tooltip_text(gettext("Reboot VM"));
    _uiManager->get_widget("/ToolBar/SuspendVm")->set_tooltip_text(gettext("Suspend VM"));
    _uiManager->get_widget("/ToolBar/ResumeVm")->set_tooltip_text(gettext("Resume VM"));
    _uiManager->get_widget("/ToolBar/SendSAS")->set_tooltip_text(gettext("Send Ctrl+Alt+Del"));
    _uiManager->get_widget("/ToolBar/ChangeCd")->set_tooltip_text(gettext("Change CD/DVD"));

    _box.pack_start(_hpaned);

    _sw1.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _sw1.set_size_request(PANE1WIDTH_DEFAULT, -1);
    _sw1.add(_serverTreeView);

    _hpaned.pack1(_sw1, false, true);
    _hpaned.pack2(_box2, true, true);

    signal_delete_event().connect(sigc::mem_fun(*this, &MainWindow::onClose));
    signal_window_state_event().connect(sigc::mem_fun(*this, &MainWindow::onWindowStateChange));
    signal_size_allocate().connect(sigc::mem_fun(*this, &MainWindow::onResize));

    _serverTreeView.signalNodeCreated().connect(sigc::mem_fun(*this, &MainWindow::onNodeCreated));
    _serverTreeView.signalSelectionChanged().connect(sigc::mem_fun(*this, &MainWindow::onHostTreeViewSelectionChanged));

    show_all_children();

    _defaultNotebook = NoContentsNotebook::create(APPNAME);
    addNotebook(_defaultNotebook);
    showNotebook(_defaultNotebook);

    updateSensitivity();
}


MainWindow::~MainWindow()
{
    Trace trace("MainWindow::dtor");

    _connObjectCreated.disconnect();
}


void MainWindow::initStockItems()
{
    _iconFactory = Gtk::IconFactory::create();

    {
        Gtk::IconSet iconSet;
        Gtk::Stock::lookup(Gtk::Stock::ADD, iconSet);
        _iconFactory->add(_stockIdAddHost, iconSet);
    }

    {
        Gtk::IconSet iconSet;
        Gtk::Stock::lookup(Gtk::Stock::NEW, iconSet);
        _iconFactory->add(_stockIdAddVm, iconSet);
    }

    {
        Gtk::IconSet iconSet;
        Gtk::Stock::lookup(Gtk::Stock::YES, iconSet);
        _iconFactory->add(_stockIdPowerOn, iconSet);
    }

    {
        Gtk::IconSet iconSet;
        Gtk::Stock::lookup(Gtk::Stock::NO, iconSet);
        _iconFactory->add(_stockIdPowerOff, iconSet);
    }

    {
        Gtk::IconSet iconSet;
        Gtk::Stock::lookup(Gtk::Stock::MEDIA_PLAY, iconSet);
        _iconFactory->add(_stockIdStartVm, iconSet);
    }

    {
        Gtk::IconSet iconSet;
        Gtk::Stock::lookup(Gtk::Stock::MEDIA_STOP, iconSet);
        _iconFactory->add(_stockIdShutdownVm, iconSet);
    }

    {
        Gtk::IconSet iconSet;
        Gtk::Stock::lookup(Gtk::Stock::REFRESH, iconSet);
        _iconFactory->add(_stockIdRebootVm, iconSet);
    }

    {
        Gtk::IconSet iconSet;
        Gtk::Stock::lookup(Gtk::Stock::MEDIA_PAUSE, iconSet);
        _iconFactory->add(_stockIdSuspendVm, iconSet);
    }

    {
        Gtk::IconSet iconSet;
        Gtk::Stock::lookup(Gtk::Stock::MEDIA_PLAY, iconSet);
        _iconFactory->add(_stockIdResumeVm, iconSet);
    }

    {
        Gtk::IconSet iconSet;
        Gtk::Stock::lookup(Gtk::Stock::CDROM, iconSet);
        _iconFactory->add(_stockIdChangeCd, iconSet);
    }

    {
        Gtk::IconSet iconSet;
        Gtk::Stock::lookup(Gtk::Stock::DIALOG_AUTHENTICATION, iconSet);
        _iconFactory->add(_stockIdAuth, iconSet);
    }

    Gtk::Stock::add(Gtk::StockItem(_stockIdAddHost, gettext("_Add host")));
    Gtk::Stock::add(Gtk::StockItem(_stockIdPowerOn, gettext("Power o_n")));
    Gtk::Stock::add(Gtk::StockItem(_stockIdPowerOff, gettext("Power o_ff")));
    Gtk::Stock::add(Gtk::StockItem(_stockIdAddVm, gettext("_Add")));
    Gtk::Stock::add(Gtk::StockItem(_stockIdStartVm, gettext("_Start")));
    Gtk::Stock::add(Gtk::StockItem(_stockIdShutdownVm, gettext("Shut do_wn")));
    Gtk::Stock::add(Gtk::StockItem(_stockIdRebootVm, gettext("_Reboot")));
    Gtk::Stock::add(Gtk::StockItem(_stockIdSuspendVm, gettext("Sus_pend")));
    Gtk::Stock::add(Gtk::StockItem(_stockIdResumeVm, gettext("Resu_me")));
    Gtk::Stock::add(Gtk::StockItem(_stockIdChangeCd, gettext("Change CD/D_VD")));
    Gtk::Stock::add(Gtk::StockItem(_stockIdAuth, gettext("Send CTR_L+ALT+DEL")));

    _iconFactory->add_default();
}


bool MainWindow::onClose(GdkEventAny* event)
{
    Controller::instance().quit();
    return true; // to stop other handlers from being invoked for this event
}


bool MainWindow::onWindowStateChange(GdkEventWindowState* event)
{
    _windowState = event->new_window_state;
    return true;
}


void MainWindow::onResize(Gtk::Allocation& a)
{
    if (!(_windowState & (GDK_WINDOW_STATE_FULLSCREEN |
                          GDK_WINDOW_STATE_MAXIMIZED |
                          GDK_WINDOW_STATE_ICONIFIED)))
    {
        _width = a.get_width();
        _height = a.get_height();
        Logger::instance().trace("MainWindow::onResize: cx=%d cy=%d", _width, _height);
    }
}


void MainWindow::onNodeCreated(RefPtr<XenObject> object)
{
    RefPtr<Notebook> notebook = NotebookFactory::create(object);
    _notebookStore.set(object, notebook);
    addNotebook(notebook);
    updateSensitivity();
    SignalManager::instance().xenObjectSignal(*object).connect(sigc::mem_fun(*this, &MainWindow::onObjectUpdated));
}


void MainWindow::onObjectUpdated(RefPtr<XenObject> object, int what)
{
    RefPtr<XenObject> xenObject = RefPtr<XenObject>::castStatic(object);
    if (what == XenObject::DESTROYED)
    {
        RefPtr<Notebook> notebook = _notebookStore.remove(xenObject);
        if (!notebook)
        {
            return;
        }
        if (notebook == _currentNotebook)
        {
            onHostTreeViewSelectionChanged();
        }
        removeNotebook(notebook);
        updateSensitivity();
    }
    else
    {
        switch (what)
        {
        case XenObject::BUSY_SET:
        case XenObject::BUSY_RESET:
        case XenObject::CONNECTED:
        case XenObject::DISCONNECTED:
        case XenObject::RECORD_UPDATED:
            updateSensitivity();
            break;
        default:
            break;
        }
    }
}


void MainWindow::onHostTreeViewSelectionChanged()
{
    RefPtr<XenObject> node;
    Model::instance().deselectAll();
    Gtk::TreeIter iter = _serverTreeView.getFirst();
    while (iter)
    {
        if (_serverTreeView.isSelected(iter))
        {
            Model::instance().select(_serverTreeView.getObject(iter));
            if (!node)
            {
                node = _serverTreeView.getObject(iter);
            }
        }
        Gtk::TreeIter iter2 = iter->children().begin();
        while (iter2)
        {
            if (_serverTreeView.isSelected(iter2))
            {
                Model::instance().select(_serverTreeView.getObject(iter2));
                if (!node)
                {
                    node = _serverTreeView.getObject(iter2);
                }
            }
            iter2++;
        }
        iter++;
    }
    RefPtr<Notebook> notebook;
    if (node)
    {
        notebook = _notebookStore.get(node);
    }
    if (!notebook)
    {
        notebook = _defaultNotebook;
    }
    showNotebook(notebook);
    updateSensitivity();
}


void MainWindow::addNotebook(RefPtr<Notebook>& notebook)
{
    notebook->hide();
    _box2.pack_start(*notebook);
}


void MainWindow::removeNotebook(RefPtr<Notebook>& notebook)
{
    if (_currentNotebook == notebook)
    {
        showNotebook(_defaultNotebook);
    }
    _box2.remove(*notebook);
}


void MainWindow::showNotebook(const RefPtr<Notebook>& notebook)
{
    if (_currentNotebook != notebook)
    {
        if (_currentNotebook)
        {
            _currentNotebook->hide();
        }
        _currentNotebook = notebook;
        if (_currentNotebook)
        {
            _currentNotebook->show();
        }
    }
}


void MainWindow::updateSensitivity()
{
    bool bRemoveHost, bConnect, bDisconnect, bTurnOnHost, bChangeHost;
    std::list<RefPtr<Host> > hosts;
    if (Model::instance().getSelected(hosts))
    {
        bConnect = true;
        bDisconnect = true;
        for (std::list<RefPtr<Host> >::iterator iter = hosts.begin(); iter != hosts.end() && (bConnect || bDisconnect); iter++)
        {
            Session& session = (*iter)->getSession();
            if (session.isConnected())
            {
                bConnect = false;
            }
            else
            {
                bDisconnect = false;
            }
        }
    }
    else
    {
        bConnect = false;
        bDisconnect = false;
    }
    bRemoveHost = hosts.size() == 1 && !hosts.front()->getSession().isConnected();
    bTurnOnHost = hosts.size() == 1 && !hosts.front()->getSession().isConnected() && !hosts.front()->getSession().getConnectSpec().mac.isNull();
    if (bTurnOnHost)
    {
        //RefPtr<Host> host = hostList.front();
        //if (host->getPing())
        //{
        //    bTurnOnHost = false;
        //}
    }
    bChangeHost = hosts.size() == 1 && hosts.front()->getSession().isConnected() && !hosts.front()->isBusy();
    bool bStartVm, bShutdownVm, bResumeVm, bChangeCd, bLive, bChangeVM;
    std::list<RefPtr<VirtualMachine> > vmList;
    if (Model::instance().getSelected(vmList))
    {
        bStartVm = true;
        bShutdownVm = true;
        bResumeVm = true;
        for (std::list<RefPtr<VirtualMachine> >::iterator iter = vmList.begin(); iter != vmList.end(); iter++)
        {
            RefPtr<VirtualMachine>& vm = *iter;
            if (vm->isBusy())
            {
                bStartVm = false;
                bShutdownVm = false;
                bResumeVm = false;
                break;
            }
            switch (vm->getRecord()->power_state)
            {
            case XEN_VM_POWER_STATE_HALTED:
                bShutdownVm = false;
                bResumeVm = false;
                break;
            case XEN_VM_POWER_STATE_PAUSED:
                bStartVm = false;
                bShutdownVm = false;
                bResumeVm = false;
                break;
            case XEN_VM_POWER_STATE_RUNNING:
                bStartVm = false;
                bResumeVm = false;
                break;
            case XEN_VM_POWER_STATE_SUSPENDED:
                bStartVm = false;
                bShutdownVm = false;
                break;
            default:
                bStartVm = false;
                bShutdownVm = false;
                bResumeVm = false;
                break;
            }
        }
    }
    else
    {
        bStartVm = false;
        bShutdownVm = false;
        bResumeVm = false;
    }
    enum xen_vm_power_state powerState = vmList.size() == 1 && !vmList.front()->isBusy() ?
        vmList.front()->getRecord()->power_state : XEN_VM_POWER_STATE_UNDEFINED;
    bChangeCd = powerState == XEN_VM_POWER_STATE_HALTED || powerState == XEN_VM_POWER_STATE_RUNNING;
    bLive = powerState == XEN_VM_POWER_STATE_RUNNING;
    bChangeVM = powerState == XEN_VM_POWER_STATE_HALTED;
    std::list<RefPtr<StorageRepository> > srList;
    Model::instance().getSelected(srList);
    bool bChangeSR = srList.size() == 1 && !srList.front()->isBusy();
    bool bSetDefaultSR = srList.size() == 1 && !srList.front()->isBusy() && srList.front()->getSubType() == StorageRepository::USR && !srList.front()->isDefault();
    bool bAddHdd = bDisconnect ? true : false;
    bool bAddSR = bChangeHost;
    bool bDeleteSR = srList.size() == 1 && !srList.front()->isBusy() && srList.front()->isCifs();
    _uiManager->get_widget("/MenuBar/File/EditConnectSpec")->set_sensitive(bRemoveHost);
    _uiManager->get_widget("/MenuBar/File/RemoveHost")->set_sensitive(bRemoveHost);
    _uiManager->get_widget("/MenuBar/Edit/Host/Connect")->set_sensitive(bConnect);
    _uiManager->get_widget("/MenuBar/Edit/Host/Disconnect")->set_sensitive(bDisconnect);
    _uiManager->get_widget("/MenuBar/Edit/Host/Change")->set_sensitive(bChangeHost);
    _uiManager->get_widget("/MenuBar/Edit/Host/TurnOn")->set_sensitive(bTurnOnHost);
    _uiManager->get_widget("/MenuBar/Edit/Host/Shutdown")->set_sensitive(bDisconnect);
    _uiManager->get_widget("/MenuBar/Edit/Host/Reboot")->set_sensitive(bDisconnect);
    _uiManager->get_widget("/MenuBar/Edit/VM/Start")->set_sensitive(bStartVm);
    _uiManager->get_widget("/MenuBar/Edit/VM/Shutdown")->set_sensitive(bShutdownVm);
    _uiManager->get_widget("/MenuBar/Edit/VM/Reboot")->set_sensitive(bShutdownVm);
    _uiManager->get_widget("/MenuBar/Edit/VM/Suspend")->set_sensitive(bShutdownVm);
    _uiManager->get_widget("/MenuBar/Edit/VM/Resume")->set_sensitive(bResumeVm);
    _uiManager->get_widget("/MenuBar/Edit/VM/Emergency")->set_sensitive(bShutdownVm);
    _uiManager->get_widget("/MenuBar/Edit/VM/ChangeCd")->set_sensitive(bChangeCd);
    _uiManager->get_widget("/MenuBar/Edit/VM/SendCtrlAltDelete")->set_sensitive(bLive);
    _uiManager->get_widget("/MenuBar/Edit/VM/Change")->set_sensitive(bChangeVM);
    _uiManager->get_widget("/MenuBar/Edit/VM/Add")->set_sensitive(bChangeHost);
    _uiManager->get_widget("/MenuBar/Edit/VM/Copy")->set_sensitive(bChangeVM);
    _uiManager->get_widget("/MenuBar/Edit/VM/Delete")->set_sensitive(bChangeVM);
    _uiManager->get_widget("/MenuBar/Edit/VM/Snapshot")->set_sensitive(bChangeVM);
    _uiManager->get_widget("/MenuBar/Edit/VM/Export")->set_sensitive(bChangeVM);
    _uiManager->get_widget("/MenuBar/Edit/VM/Import")->set_sensitive(bChangeHost);
    _uiManager->get_widget("/MenuBar/Edit/VM/Verify")->set_sensitive(true);
    _uiManager->get_widget("/MenuBar/Edit/SR/Change")->set_sensitive(bChangeSR);
    _uiManager->get_widget("/MenuBar/Edit/SR/Change/Default")->set_sensitive(bSetDefaultSR);
    _uiManager->get_widget("/MenuBar/Edit/SR/AddHdd")->set_sensitive(bAddHdd);
    _uiManager->get_widget("/MenuBar/Edit/SR/AddCIFS")->set_sensitive(bAddSR);
    _uiManager->get_widget("/MenuBar/Edit/SR/DeleteCifs")->set_sensitive(bDeleteSR);
    if (!hosts.size())
    {
        _uiManager->get_widget("/ToolBar/AddHost")->show();
        _uiManager->get_widget("/ToolBar/Connect")->hide();
        _uiManager->get_widget("/ToolBar/Disconnect")->hide();
    }
    else if (bConnect)
    {
        _uiManager->get_widget("/ToolBar/AddHost")->hide();
        _uiManager->get_widget("/ToolBar/Connect")->show();
        _uiManager->get_widget("/ToolBar/Connect")->set_sensitive(true);
        _uiManager->get_widget("/ToolBar/Disconnect")->hide();
    }
    else if (bDisconnect)
    {
        _uiManager->get_widget("/ToolBar/AddHost")->hide();
        _uiManager->get_widget("/ToolBar/Connect")->hide();
        _uiManager->get_widget("/ToolBar/Disconnect")->show();
        _uiManager->get_widget("/ToolBar/Disconnect")->set_sensitive(true);
    }
    else
    {
        _uiManager->get_widget("/ToolBar/AddHost")->hide();
        _uiManager->get_widget("/ToolBar/Connect")->show();
        _uiManager->get_widget("/ToolBar/Connect")->set_sensitive(false);
        _uiManager->get_widget("/ToolBar/Disconnect")->hide();
    }
    _uiManager->get_widget("/ToolBar/AddVm")->set_sensitive(bChangeHost);
    _uiManager->get_widget("/ToolBar/StartVm")->set_sensitive(bStartVm);
    _uiManager->get_widget("/ToolBar/ShutdownVm")->set_sensitive(bShutdownVm);
    _uiManager->get_widget("/ToolBar/RebootVm")->set_sensitive(bShutdownVm);
    _uiManager->get_widget("/ToolBar/SuspendVm")->set_sensitive(bShutdownVm);
    _uiManager->get_widget("/ToolBar/SendSAS")->set_sensitive(bShutdownVm);
    _uiManager->get_widget("/ToolBar/ChangeCd")->set_sensitive(bChangeCd);
    if (bResumeVm)
    {
        _uiManager->get_widget("/ToolBar/StartVm")->hide();
        _uiManager->get_widget("/ToolBar/ResumeVm")->show();
    }
    else
    {
        _uiManager->get_widget("/ToolBar/StartVm")->show();
        _uiManager->get_widget("/ToolBar/ResumeVm")->hide();
    }
}


void MainWindow::setSize(int cx, int cy)
{
    set_default_size(cx, cy);
}


int MainWindow::getPane1Width() const
{
    return _sw1.get_width();
}


void MainWindow::setPane1Width(int cx)
{
    _sw1.set_size_request(cx, -1);
}


void MainWindow::clear()
{
    Trace trace("MainWindow::clear");
    _serverTreeView.clear();
    _defaultNotebook.clear();
    _currentNotebook.clear();
    _notebookStore.clear();
}
