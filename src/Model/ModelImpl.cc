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
