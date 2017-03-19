// Copyright (C) 2012-2017 Hideaki Narita


#include "Base/StringBuffer.h"
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
    : XenObject(XenObject::TASK, session, reinterpret_cast<char*>(handle))
    , _status(XEN_TASK_STATUS_TYPE_UNDEFINED)
    , _object(RefPtr<XenObject>(object, 1))
    , _progress(0.0)
{
    if (messageOnFailure)
    {
        _messageOnFailure = messageOnFailure;
    }
    if (messageOnSuccess)
    {
        _messageOnSuccess = messageOnSuccess;
    }
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
    Glib::Mutex::Lock k(_mutex);
    _cond.wait(_mutex);
}


bool XenTask::wait(const Glib::TimeVal& absTime)
{
    Glib::Mutex::Lock k(_mutex);
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
