// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_XENTASK_H
#define HNRT_XENTASK_H


#include <glibmm.h>
#include "Api.h"
#include "XenObject.h"


namespace hnrt
{
    class XenTask
        : public XenObject
    {
    public:

        static RefPtr<XenTask> create(Session&, xen_task, XenObject*, const char* messageOnFailure = NULL, const char* messageOnSuccess = NULL);

        const XenObject& getObject() const { return *_object; }
        XenObject& getObject() { return *_object; }
        xen_task_status_type getStatus() const { return _status; }
        void setStatus(xen_task_status_type value) { _status = value; }
        const Glib::ustring& getErrorMessage() const { return _errorMessage; }
        void setErrorMessage(const char* value) { _errorMessage = value; }
        void setProgress(double);
        const Glib::ustring& getMessageOnSuccess() const { return _messageOnSuccess; }
        void setMessageOnSuccess(const char* value) { _messageOnSuccess = value; }
        const Glib::ustring& getMessageOnFailure() const { return _messageOnFailure; }
        void setMessageOnFailure(const char* value) { _messageOnFailure = value; }
        void wait();
        bool wait(const Glib::TimeVal&);
        void broadcast();
        void signal();
        virtual void onSuccess() {}
        virtual void onFailure() {}

    protected:

        XenTask(Session&, xen_task, XenObject*, const char* messageOnFailure, const char* messageOnSuccess);
        XenTask(const XenTask&);
        void operator =(const XenTask&);

        xen_task_status_type _status;
        Glib::ustring _errorMessage;
        RefPtr<XenObject> _object;
        double _progress;
        Glib::ustring _messageOnSuccess;
        Glib::ustring _messageOnFailure;
        Glib::Cond _cond;
    };
}


#endif //!HNRT_XENTASK_H
