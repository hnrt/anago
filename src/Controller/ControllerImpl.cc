// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Net/PingAgent.h"
#include "Thread/ThreadManager.h"
#include "View/View.h"
#include "XenServer/Host.h"
#include "XenServer/Session.h"
#include "XenServer/XenObjectStore.h"
#include "XenServer/XenTask.h"
#include "ControllerImpl.h"
#include "SignalManager.h"


using namespace hnrt;


ControllerImpl::ControllerImpl()
    : _quitInProgress(false)
{
    Trace trace("ControllerImpl::ctor");

    SignalManager& sm = SignalManager::instance();
    sm.xenObjectSignal(XenObject::CONNECT_FAILED).connect(sigc::mem_fun(*this, &ControllerImpl::onConnectFailed));
    sm.xenObjectSignal(XenObject::ERROR).connect(sigc::mem_fun(*this, &ControllerImpl::onXenObjectError));
    sm.xenObjectSignal(XenObject::TASK_ON_SUCCESS).connect(sigc::mem_fun(*this, &ControllerImpl::onXenTaskUpdated));
    sm.xenObjectSignal(XenObject::TASK_ON_FAILURE).connect(sigc::mem_fun(*this, &ControllerImpl::onXenTaskUpdated));
    sm.xenObjectSignal(XenObject::TASK_ON_CANCELLED).connect(sigc::mem_fun(*this, &ControllerImpl::onXenTaskUpdated));
    sm.xenObjectSignal(XenObject::TASK_IN_PROGRESS).connect(sigc::mem_fun(*this, &ControllerImpl::onXenTaskUpdated));
}


ControllerImpl::~ControllerImpl()
{
    Trace trace("ControllerImpl::dtor");
}


void ControllerImpl::parseCommandLine(int argc, char *argv[])
{
    Trace trace("ControllerImpl::parseCommandLine");

    for (int index = 1; index < argc; index++)
    {
        if (!strcmp(argv[index], "-log"))
        {
            if (++index == argc || argv[index][0] == '-')
            {
                static char msg[256];
                snprintf(msg, sizeof(msg), "Command line option -log operand was not specified.");
                throw std::runtime_error(msg);
            }
            Logger::instance().setLevel(LogLevel::parse(argv[index]));
        }
        else
        {
            static char msg[256];
            snprintf(msg, sizeof(msg), "Bad command line syntax: %s", argv[index]);
            throw std::runtime_error(msg);
        }
    }

    PingAgent::instance().open();

    connectAtStartup();
}


void ControllerImpl::quit()
{
    Trace trace("ControllerImpl::quit");

    if (!_quitInProgress)
    {
        _quitInProgress = true;
        View::instance().getWindow().set_title(gettext("Quitting..."));
        PingAgent::instance().close();
        if (quit2())
        {
            Glib::signal_timeout().connect(sigc::mem_fun(*this, &ControllerImpl::quit2), 100); // 100 milleseconds
        }
    }
}


bool ControllerImpl::quit2()
{
    Trace trace("ControllerImpl::quit2");

    int busyCount = 0;
    std::list<RefPtr<Host> > hosts;
    Model::instance().get(hosts);
    for (std::list<RefPtr<Host> >::iterator iter = hosts.begin(); iter != hosts.end(); iter++)
    {
        RefPtr<Host>& host = *iter;
        if (host->isBusy())
        {
            busyCount++;
        }
        else
        {
            Session& session = host->getSession();
            if (session.isConnected())
            {
                if (session.disconnect())
                {
                    host->onDisconnected();
                }
            }
        }
    }
    hosts.clear();
    int backgroundCount = ThreadManager::instance().count();
    if (busyCount || backgroundCount)
    {
        trace.put("busy=%d background=%d", busyCount, backgroundCount);
        return true; // to be invoked again
    }
    else
    {
        View::instance().getWindow().hide();
        return false; // done
    }
}


void ControllerImpl::onConnectFailed(RefPtr<XenObject> object, int what)
{
    StringBuffer message;
    {
        Session& session = object->getSession();
        Session::Lock lock(session);
        message.format(gettext("Failed to connect to %s.\n"), session.getConnectSpec().hostname.c_str());
        XenServer::getError(session, message, "\n");
        session.clearError();
        disconnect(session.getStore().getHost());
    }
    View::instance().showWarning(message.str());
}


void ControllerImpl::onXenObjectError(RefPtr<XenObject> object, int what)
{
    StringBuffer message;
    {
        Session& session = object->getSession();
        Session::Lock lock(session);
        XenServer::getError(session, message, "\n");
        session.clearError();
    }
    View::instance().showWarning(message.str());
}


void ControllerImpl::onXenTaskUpdated(RefPtr<XenObject> object, int what)
{
    RefPtr<XenTask> task = RefPtr<XenTask>::castStatic(object);
    switch (what)
    {
    case XenObject::TASK_ON_SUCCESS:
    {
        StringBuffer message;
        message += task->getMessageOnSuccess().c_str();
        if (message.len())
        {
            View::instance().showInfo(message.str());
        }
        task->onSuccess();
        break;
    }
    case XenObject::TASK_ON_FAILURE:
    {
        StringBuffer message;
        XenServer::getErrorFromTask(task->getSession(), task->getHandle(), message, "\n");
        task->setErrorMessage(message);
        message.clear();
        message += task->getMessageOnFailure().c_str();
        if (message.len())
        {
            message += "\n";
            message += task->getErrorMessage().c_str();
            View::instance().showWarning(message.str());
        }
        task->onFailure();
        break;
    }
    case XenObject::TASK_ON_CANCELLED:
    {
        task->onFailure();
        break;
    }
    case XenObject::TASK_IN_PROGRESS:
    {
        break;
    }
    default:
        break;
    }
}


void ControllerImpl::openVmStatusWindow()
{
    //TODO: IMPLEMENT
}


void ControllerImpl::showAbout()
{
    View::instance().about();
}
