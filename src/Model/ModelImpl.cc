// Copyright (C) 2012-2017 Hideaki Narita


#include <stdlib.h>
#include "App/Constants.h"
#include "Logger/Trace.h"
#include "XenServer/Host.h"
#include "XenServer/Session.h"
#include "XenServer/XenObjectStore.h"
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
}


void ModelImpl::fini()
{
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
            return;
        }
    }
    RefPtr<Host> host = Host::create(cs);
    Session& session = host->getSession();
    session.getStore().add(host);
    _hosts.push_back(host);
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
            session.getStore().removeHost();
            return;
        }
    }
}


void ModelImpl::removeAllSessions()
{
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


RefPtr<PatchBase> ModelImpl::getPatchBase()
{
    Glib::RecMutex::Lock lock(_mutex);
    return _patchBase;
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
