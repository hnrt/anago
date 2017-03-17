// Copyright (C) 2012-2017 Hideaki Narita


#include <stdlib.h>
#include "App/Constants.h"
#include "Logger/Trace.h"
#include "XenServer/Session.h"
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


int ModelImpl::get(std::list<Session*>& list)
{
    Glib::RecMutex::Lock lock(_mutex);
    int count = 0;
    for (std::list<Session*>::const_iterator iter = _sessions.begin(); iter != _sessions.end(); iter++)
    {
        list.push_back(*iter);
    }
    return count;
}


void ModelImpl::add(const ConnectSpec& cs)
{
    Glib::RecMutex::Lock lock(_mutex);
    for (std::list<Session*>::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++)
    {
        Session& session = **iter;
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
    _sessions.push_back(new Session(cs));
}


void ModelImpl::ModelImpl::remove(Session& session)
{
    Glib::RecMutex::Lock lock(_mutex);
    for (std::list<Session*>::iterator iter = _sessions.begin(); iter != _sessions.end(); iter++)
    {
        if (**iter == session)
        {
            _sessions.erase(iter);
            session.disconnect();
            session.unreference();
            return;
        }
    }
}


void ModelImpl::removeAllSessions()
{
    Glib::RecMutex::Lock lock(_mutex);
    while (!_sessions.empty())
    {
        Session& session = *_sessions.front();
        _sessions.pop_front();
        session.disconnect();
        session.unreference();
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


int ModelImpl::getSelected(std::list<Session*>& list)
{
    Glib::RecMutex::Lock lock(_mutex);
    std::list<Session*>::size_type count0 = list.size();
    for (std::list<RefPtr<XenObject> >::iterator iter = _selected.begin(); iter != _selected.end(); iter++)
    {
        RefPtr<XenObject>& object = *iter;
        list.push_back(&object->getSession());
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
