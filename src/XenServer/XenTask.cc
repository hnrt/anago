// Copyright (C) 2012-2017 Hideaki Narita


#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Util/Util.h"
#include "Session.h"
#include "XenObjectStore.h"
#include "XenTask.h"


using namespace hnrt;


RefPtr<XenTask> XenTask::create(Session& session, xen_task handle, XenObject* object, const char* messageOnFailure, const char* messageOnSuccess)
{
    RefPtr<XenTask> task(new XenTask(session, handle, object, messageOnFailure, messageOnSuccess));
    session.getStore().add(task);
    return task;
}


XenTask::XenTask(Session& session, xen_task handle, XenObject* object, const char* messageOnFailure, const char* messageOnSuccess)
    : XenObject(XenObject::TASK, session, handle, NULL, NULL)
    , _status(XEN_TASK_STATUS_TYPE_UNDEFINED)
    , _object(RefPtr<XenObject>(object, 1))
    , _progress(0.0)
{
    Trace trace(StringBuffer().format("TASK@%zx::ctor", this), "%s", _refid.c_str());
    if (messageOnFailure)
    {
        _messageOnFailure = messageOnFailure;
    }
    if (messageOnSuccess)
    {
        _messageOnSuccess = messageOnSuccess;
    }
}


XenTask::~XenTask()
{
    Trace trace(StringBuffer().format("TASK@%zx::dtor", this), "%s", _refid.c_str());
}


void XenTask::setProgress(double value)
{
    if (_progress != value)
    {
        _progress = value;
        StringBuffer sb;
        FormatProgress(sb, _object->getDisplayStatus().c_str(), _progress * 100);
        _object->setDisplayStatus(sb);
    }
}


void XenTask::wait()
{
    Glib::Mutex::Lock lock(_mutex);
    _cond.wait(_mutex);
}


bool XenTask::wait(const Glib::TimeVal& absTime)
{
    Glib::Mutex::Lock lock(_mutex);
    return _cond.timed_wait(_mutex, absTime);
}


void XenTask::broadcast()
{
    _cond.broadcast();
}


void XenTask::signal()
{
    _cond.signal();
}
