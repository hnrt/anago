// Copyright (C) 2012-2017 Hideaki Narita


#include <stdlib.h>
#include "App/Constants.h"
#include "Logger/Trace.h"
#include "XenServer/Session.h"
#include "ModelImpl.h"


using namespace hnrt;


static Glib::ustring GetConfigPath()
{
    return Glib::ustring::compose("%1/.%2.conf",
                                  getenv("HOME"),
                                  APPNAME);
}


ModelImpl::ModelImpl()
    : _path(GetConfigPath())
    , _width(640)
    , _height(480)
    , _panelWidth(200)
{
    Trace trace(__PRETTY_FUNCTION__);
}


ModelImpl::~ModelImpl()
{
    Trace trace(__PRETTY_FUNCTION__);
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


int ModelImpl::getPanelWidth()
{
    Glib::RecMutex::Lock lock(_mutex);
    return _panelWidth;
}


void ModelImpl::setPanelWidth(int value)
{
    Glib::RecMutex::Lock lock(_mutex);
    _panelWidth = value;
}
