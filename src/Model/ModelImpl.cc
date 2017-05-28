// Copyright (C) 2012-2017 Hideaki Narita


#include <stdlib.h>
#include "App/Constants.h"
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Net/Console.h"
#include "Net/PingAgent.h"
#include "Util/UUID.h"
#include "XenServer/Host.h"
#include "XenServer/Session.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/VirtualMachine.h"
#include "XenServer/XenObjectStore.h"
#include "XenServer/XenObjectTypeMap.h"
#include "ModelImpl.h"
#include "PatchBase.h"


using namespace hnrt;


static Glib::ustring GetConfigPath()
{
    return Glib::ustring::compose("%1/.%2.conf",
                                  getenv("HOME"),
                                  APPNAME);
}


static Glib::ustring GetAppDir()
{
    return Glib::ustring::compose("%1/.%2.d/",
                                  getenv("HOME"),
                                  APPNAME);
}


ModelImpl::ModelImpl()
    : _path(GetConfigPath())
    , _appDir(GetAppDir())
    , _patchBase(PatchBase::create())
    , _width(WIDTH_DEFAULT)
    , _height(HEIGHT_DEFAULT)
    , _pane1Width(PANE1WIDTH_DEFAULT)
{
    Trace trace("ModelImpl::ctor");
}


ModelImpl::~ModelImpl()
{
    Trace trace("ModelImpl::dtor");
}


void ModelImpl::init()
{
    _patchBase->init();
    XenObjectTypeMap::init();
}


void ModelImpl::fini()
{
    XenObjectTypeMap::fini();
    _patchBase->fini();
}


void ModelImpl::clear()
{
    Trace trace("ModelImpl::clear");
    deselectAll();
    removeAllSessions();
}


int ModelImpl::get(std::list<RefPtr<Host> >& list)
{
    Glib::RecMutex::Lock lock(_mutex);
    int count = 0;
    for (std::list<RefPtr<Host> >::iterator iter = _hosts.begin(); iter != _hosts.end(); iter++)
    {
        list.push_back(*iter);
    }
    return count;
}


void ModelImpl::add(const ConnectSpec& cs)
{
    Glib::RecMutex::Lock k(_mutex);
    if (!cs.uuid.empty())
    {
        for (std::list<RefPtr<Host> >::iterator iter = _hosts.begin(); iter != _hosts.end(); iter++)
        {
            RefPtr<Host>& host = *iter;
            Session& session = host->getSession();
            ConnectSpec& cs2 = session.getConnectSpec();
            if (cs2.uuid == cs.uuid)
            {
                cs2.displayname = cs.displayname;
                cs2.hostname = cs.hostname;
                cs2.username = cs.username;
                cs2.password = cs.password;
                cs2.autoConnect = cs.autoConnect;
                host->emit(XenObject::NAME_UPDATED);
                return;
            }
        }
    }
    RefPtr<Host> host = Host::create(cs);
    if (host->getSession().getConnectSpec().uuid.empty())
    {
        host->getSession().getConnectSpec().uuid = UUID::generate();
    }
    _hosts.push_back(host);
    PingAgent::instance().add(host->getSession().getConnectSpec().hostname.c_str());
}


void ModelImpl::ModelImpl::remove(Session& session)
{
    Glib::RecMutex::Lock k(_mutex);
    for (std::list<RefPtr<Host> >::iterator iter = _hosts.begin(); iter != _hosts.end(); iter++)
    {
        RefPtr<Host>& host = *iter;
        if (host->getSession() == session)
        {
            _hosts.erase(iter);
            if (session.isConnected())
            {
                if (session.disconnect())
                {
                    host->onDisconnected();
                }
            }
            PingAgent::instance().remove(session.getConnectSpec().hostname.c_str());
            session.getStore().removeHost();
            return;
        }
    }
}


void ModelImpl::removeAllSessions()
{
    PingAgent::instance().clear();
    Glib::RecMutex::Lock k(_mutex);
    while (!_hosts.empty())
    {
        RefPtr<Host> host = _hosts.front();
        _hosts.pop_front();
        Session& session = host->getSession();
        if (session.isConnected())
        {
            if (session.disconnect())
            {
                session.disconnect();
            }
        }
        session.getStore().removeHost();
        Logger::instance().trace("ModelImpl::removeAllSessions: host=%zx count=%d", host.ptr(), host->refCount());
    }
}


void ModelImpl::deselectAll()
{
    Glib::RecMutex::Lock lock(_mutex);
    _selected.clear();
}


void ModelImpl::select(const RefPtr<XenObject>& object)
{
    Glib::RecMutex::Lock lock(_mutex);
    _selected.push_back(object);
}


int ModelImpl::getSelected(std::list<RefPtr<Host> >& list)
{
    Glib::RecMutex::Lock lock(_mutex);
    std::list<RefPtr<Host> >::size_type count0 = list.size();
    for (std::list<RefPtr<XenObject> >::iterator iter = _selected.begin(); iter != _selected.end(); iter++)
    {
        RefPtr<XenObject>& object = *iter;
        RefPtr<Host> host = object->getSession().getStore().getHost();
        for (std::list<RefPtr<Host> >::iterator iter2 = list.begin();; iter2++)
        {
            if (iter2 == list.end())
            {
                list.push_back(host);
                break;
            }
            else if ((*iter2).ptr() == host.ptr())
            {
                break;
            }
        }
    }
    return static_cast<int>(list.size() - count0);
}


int ModelImpl::getSelected(std::list<RefPtr<VirtualMachine> >& list)
{
    Glib::RecMutex::Lock lock(_mutex);
    std::list<RefPtr<VirtualMachine> >::size_type count0 = list.size();
    for (std::list<RefPtr<XenObject> >::iterator iter = _selected.begin(); iter != _selected.end(); iter++)
    {
        RefPtr<XenObject>& object = *iter;
        if (object->getType() != XenObject::VM)
        {
            continue;
        }
        RefPtr<VirtualMachine> vm = RefPtr<VirtualMachine>::castStatic(object);
        for (std::list<RefPtr<VirtualMachine> >::iterator iter2 = list.begin();; iter2++)
        {
            if (iter2 == list.end())
            {
                list.push_back(vm);
                break;
            }
            else if ((*iter2).ptr() == vm.ptr())
            {
                break;
            }
        }
    }
    return static_cast<int>(list.size() - count0);
}


int ModelImpl::getSelected(std::list<RefPtr<StorageRepository> >& list)
{
    Glib::RecMutex::Lock lock(_mutex);
    std::list<RefPtr<StorageRepository> >::size_type count0 = list.size();
    for (std::list<RefPtr<XenObject> >::iterator iter = _selected.begin(); iter != _selected.end(); iter++)
    {
        RefPtr<XenObject>& object = *iter;
        if (object->getType() != XenObject::SR)
        {
            continue;
        }
        RefPtr<StorageRepository> sr = RefPtr<StorageRepository>::castStatic(object);
        for (std::list<RefPtr<StorageRepository> >::iterator iter2 = list.begin();; iter2++)
        {
            if (iter2 == list.end())
            {
                list.push_back(sr);
                break;
            }
            else if ((*iter2).ptr() == sr.ptr())
            {
                break;
            }
        }
    }
    return static_cast<int>(list.size() - count0);
}


RefPtr<Host> ModelImpl::getSelectedHost()
{
    std::list<RefPtr<Host> > list;
    if (getSelected(list) == 1)
    {
        return list.front();
    }
    else
    {
        return RefPtr<Host>();
    }
}


RefPtr<VirtualMachine> ModelImpl::getSelectedVm()
{
    std::list<RefPtr<VirtualMachine> > list;
    if (getSelected(list) == 1)
    {
        return list.front();
    }
    else
    {
        return RefPtr<VirtualMachine>();
    }
}


RefPtr<StorageRepository> ModelImpl::getSelectedSr()
{
    std::list<RefPtr<StorageRepository> > list;
    if (getSelected(list) == 1)
    {
        return list.front();
    }
    else
    {
        return RefPtr<StorageRepository>();
    }
}


void ModelImpl::selectSnapshot(const RefPtr<VirtualMachine>& vm)
{
    Glib::RecMutex::Lock lock(_mutex);
    _selectedSnapshot = vm;
}


void ModelImpl::deselectSnapshot()
{
    Glib::RecMutex::Lock lock(_mutex);
    _selectedSnapshot.reset();
}


RefPtr<VirtualMachine> ModelImpl::getSelectedSnapshot()
{
    Glib::RecMutex::Lock lock(_mutex);
    return _selectedSnapshot;
}


RefPtr<PatchBase> ModelImpl::getPatchBase()
{
    return _patchBase;
}


void ModelImpl::addConsole(const Glib::ustring& uuid, const RefPtr<Console>& console)
{
    Glib::RecMutex::Lock lock(_mutex);
    getConsoleInfo(uuid).console = console;
}


void ModelImpl::removeConsole(const Glib::ustring& uuid)
{
    Glib::RecMutex::Lock lock(_mutex);
    getConsoleInfo(uuid).console.reset();
}


RefPtr<Console> ModelImpl::getConsole(const Glib::ustring& uuid)
{
    Glib::RecMutex::Lock lock(_mutex);
    return getConsoleInfo(uuid).console;
}


int ModelImpl::getWidth()
{
    Glib::RecMutex::Lock lock(_mutex);
    return _width;
}


void ModelImpl::setWidth(int value)
{
    Glib::RecMutex::Lock lock(_mutex);
    _width = value;
}


int ModelImpl::getHeight()
{
    Glib::RecMutex::Lock lock(_mutex);
    return _height;
}


void ModelImpl::setHeight(int value)
{
    Glib::RecMutex::Lock lock(_mutex);
    _height = value;
}


int ModelImpl::getPane1Width()
{
    Glib::RecMutex::Lock lock(_mutex);
    return _pane1Width;
}


void ModelImpl::setPane1Width(int value)
{
    Glib::RecMutex::Lock lock(_mutex);
    _pane1Width = value;
}


ConsoleInfo& ModelImpl::getConsoleInfo(const Glib::ustring& uuid)
{
    ConsoleMap::iterator iter = _consoleMap.find(uuid);
    if (iter == _consoleMap.end())
    {
        ConsoleInfo info(uuid);
        _consoleMap.insert(ConsoleEntry(info.uuid, info));
        iter = _consoleMap.find(uuid);
    }
    return iter->second;
}


bool ModelImpl::getConsoleEnabled(const Glib::ustring& uuid)
{
    Glib::RecMutex::Lock lock(_mutex);
    return getConsoleInfo(uuid).enabled;
}


void ModelImpl::setConsoleEnabled(const Glib::ustring& uuid, bool value)
{
    Glib::RecMutex::Lock lock(_mutex);
    getConsoleInfo(uuid).enabled = value;
}


bool ModelImpl::getConsoleScale(const Glib::ustring& uuid)
{
    Glib::RecMutex::Lock lock(_mutex);
    return getConsoleInfo(uuid).scale;
}


void ModelImpl::setConsoleScale(const Glib::ustring& uuid, bool value)
{
    Glib::RecMutex::Lock lock(_mutex);
    getConsoleInfo(uuid).scale = value;
}


Glib::ustring ModelImpl::getExportVmPath(const VirtualMachine& vm)
{
    Glib::RecMutex::Lock lock(_mutex);
    StringBuffer path;
    if (_exportVmPath.empty())
    {
        path = getenv("HOME");
        path += "/";
    }
    else
    {
        Glib::ustring::size_type pos = _exportVmPath.rfind('/');
        if (pos == Glib::ustring::npos)
        {
            path = "./";
        }
        else
        {
            path.assign(_exportVmPath.c_str(), pos + 1);
        }
    }
    Glib::ustring name = vm.getName();
    if (name.empty())
    {
        path += "No Name";
    }
    else
    {
        for (const char* s = name.c_str(); *s; s++)
        {
            if (*s == '/')
            {
                path += "\xEF\xBC\x8F"; // fullwidth solidus U+FF0F
            }
            else
            {
                path += *s;
            }
        }
    }
    path += ".xva";
    return Glib::ustring(path);
}


void ModelImpl::setExportVmPath(const Glib::ustring& path)
{
    Glib::RecMutex::Lock lock(_mutex);
    _exportVmPath = path;
}


bool ModelImpl::getExportVmVerify()
{
    Glib::RecMutex::Lock lock(_mutex);
    return _exportVmVerify;
}


void ModelImpl::setExportVmVerify(bool value)
{
    Glib::RecMutex::Lock lock(_mutex);
    _exportVmVerify = value;
}


Glib::ustring ModelImpl::getImportVmPath()
{
    Glib::RecMutex::Lock lock(_mutex);
    return _importVmPath;
}


void ModelImpl::setImportVmPath(const Glib::ustring& path)
{
    Glib::RecMutex::Lock lock(_mutex);
    _importVmPath = path;
}


Glib::ustring ModelImpl::getVerifyVmPath()
{
    Glib::RecMutex::Lock lock(_mutex);
    return _verifyVmPath;
}


void ModelImpl::setVerifyVmPath(const Glib::ustring& path)
{
    Glib::RecMutex::Lock lock(_mutex);
    _verifyVmPath = path;
}


void ModelImpl::getCifsSpec(CifsSpec& spec)
{
    spec = _cifsSpec;
}


void ModelImpl::setCifsSpec(const CifsSpec& spec)
{
    _cifsSpec = spec;
}
