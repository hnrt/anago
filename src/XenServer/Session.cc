// Copyright (C) 2012-2017 Hideaki Narita


#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <curl/curl.h>
#include "Base/Atomic.h"
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Session.h"
#include "XenObjectStore.h"
#include "Api.h"
#include "Constants.h"


using namespace hnrt;


struct xen_comms
{
    xen_result_func func;
    void* handle;
    //StringBuffer* pbuf;
};


static size_t rpcReceive(void* ptr, size_t size, size_t nmemb, xen_comms* pcomms)
{
    size_t n = size * nmemb;
    //pcomms->pbuf->append((const char*)ptr, n);
    size_t ret = pcomms->func(ptr, n, pcomms->handle) ? n : 0;
    return ret;
}


static int rpcExecute(const void* data, size_t size, void* user_handle, void* result_handle, xen_result_func result_func)
{
    Session* x = (Session*)user_handle;

    CURL* curl = curl_easy_init();
    if (curl == NULL)
    {
        Logger::instance().error("CURL failed.");
        return -1;
    }

    //StringBuffer buf;

    xen_comms comms = {
        result_func,
        result_handle,
        //&buf
    };

    curl_easy_setopt(curl, CURLOPT_URL, x->url());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, rpcReceive);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &comms);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

    CURLcode result = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return result;
}


Session::Session(const ConnectSpec& cs)
    : XenObject(XenObject::SESSION, *this, NULL, NULL, NULL)
    , _connectSpec(cs)
    , _ptr(NULL)
    , _state(NONE)
    , _objectStore(new XenObjectStore)
    , _monitoring(false)
{
    Trace trace(StringBuffer().format("Session@%zx::ctor", this),
                "display=%s hostname=%s username=%s", cs.displayname.c_str(), cs.hostname.c_str(), cs.username.c_str());
}


Session::Session()
    : XenObject(XenObject::SESSION, *this, NULL, NULL, NULL)
    , _connectSpec()
    , _ptr(NULL)
    , _state(NONE)
    , _monitoring(false)
{
    Trace trace(StringBuffer().format("Session@%zx::ctor", this));
}


Session::~Session()
{
    Trace trace(StringBuffer().format("Session@%zx::dtor", this));
    if (_state != NONE)
    {
        disconnect();
    }
}


bool Session::isConnected() const
{
    return (_state == PRIMARY || _state == SECONDARY) ? true : false;
}


bool Session::connect()
{
    Trace trace(StringBuffer().format("Session@%zx::connect", this));
    if (_state != NONE)
    {
        disconnect();
    }
    _state = PRIMARY_PENDING;
    _url = Glib::ustring::compose("https://%1", _connectSpec.hostname);
    Glib::ustring pw = _connectSpec.descramblePassword();
    _ptr = xen_session_login_with_password(rpcExecute, this, _connectSpec.username.c_str(), pw.c_str(), xen_api_latest_version);
    pw.clear();
    if (_ptr->ok)
    {
        _state = PRIMARY;
    }
    trace.put("return=%d", _ptr->ok);
    return _ptr->ok;
}


bool Session::connect(const Session& other)
{
    Trace trace(StringBuffer().format("Session@%zx::connect", this));
    if (_state != NONE)
    {
        disconnect();
    }
    _state = SECONDARY_PENDING;
    _connectSpec = other._connectSpec;
    _url = Glib::ustring::compose("https://%1", _connectSpec.hostname);
    Glib::ustring pw = _connectSpec.descramblePassword();
    _ptr = xen_session_slave_local_login_with_password(rpcExecute, this, _connectSpec.username.c_str(), pw.c_str());
    pw.clear();
    if (_ptr->ok)
    {
        _state = SECONDARY;
        _objectStore = other._objectStore;
    }
    trace.put("return=%d", _ptr->ok);
    return _ptr->ok;
}


bool Session::disconnect()
{
    Trace trace(StringBuffer().format("Session@%zx::disconnect", this));
    bool retval = false;
    switch (InterlockedExchange((int32_t*)&_state, NONE))
    {
    case NONE:
        break;
    case PRIMARY_PENDING: // This is the case that primary connect operation failed.
        _objectStore->clear();
        xen_session_logout(_ptr);
        _ptr = NULL;
        break;
    case PRIMARY:
        {
            xen_task task;
            if (xen_task_create(_ptr, &task, (char*)"disconnect", (char*)"a dummy task to disconnect secondary session"))
            {
                xen_task_destroy(_ptr, task);
                xen_task_free(task);
            }
        }
        // wait until the monitor ends or 1 second elapses
        for (int i = 0; _monitoring && i < 1000; i++)
        {
            struct timespec t1, t2;
            t1.tv_sec = 0;
            t1.tv_nsec = 1000000;
            t2.tv_sec = 0;
            t2.tv_nsec = 0;
            nanosleep(&t1, &t2);
        }
        _objectStore->clear();
        xen_session_logout(_ptr);
        _ptr = NULL;
        retval = true;
        break;
    case SECONDARY_PENDING: // This is the case that secondary connect operation failed.
        xen_session_local_logout(_ptr);
        _ptr = NULL;
        break;
    case SECONDARY:
        xen_session_local_logout(_ptr);
        _ptr = NULL;
        retval = true;
        break;
    default:
        break;
    }
    trace.put("return=%d", retval);
    return retval;
}


bool Session::succeeded()
{
    return _ptr && _ptr->ok ? true : false; 
}


bool Session::failed()
{
    return _ptr && !_ptr->ok ? true : false; 
}


void Session::clearError()
{
    if (_ptr)
    {
        xen_session_clear_error(_ptr);
    }
}


bool Session::hasError()
{
    return _ptr && !_ptr->ok &&
        _ptr->error_description_count &&
        _ptr->error_description &&
        _ptr->error_description[0] ? true : false;
}


bool Session::hasError(const char* error)
{
    return _ptr && !_ptr->ok &&
        _ptr->error_description_count &&
        _ptr->error_description &&
        _ptr->error_description[0] &&
        !strcmp(_ptr->error_description[0], error) ? true : false;
}


bool Session::operator ==(const Session& rhs) const
{
    if (_ptr)
    {
        return _ptr == rhs._ptr;
    }
    else
    {
        return
            _connectSpec.displayname == rhs._connectSpec.displayname &&
            _connectSpec.hostname == rhs._connectSpec.hostname &&
            _connectSpec.username == rhs._connectSpec.username &&
            _connectSpec.password == rhs._connectSpec.password;
    }
}
