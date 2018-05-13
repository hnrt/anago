// Copyright (C) 2012-2018 Hideaki Narita


#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include "Base/Atomic.h"
#include "Logger/Trace.h"
#include "Protocol/HttpClient.h"
#include "Api.h"
#include "Constants.h"
#include "Session.h"
#include "XenObjectStore.h"


using namespace hnrt;


static bool rpcWrite(const void* ptr, size_t len, void* handle, xen_result_func func)
{
    return func(ptr, len, handle) ? true : false;
}


static int rpcExecute(const void* data, size_t size, void* user_handle, void* result_handle, xen_result_func result_func)
{
    Session* pThis = reinterpret_cast<Session*>(user_handle);

    RefPtr<HttpClient> httpClient = HttpClient::create();
    httpClient->init();
    httpClient->setUrl(pThis->url());
    httpClient->setPost(data, size);
    httpClient->setWriteFunction(sigc::bind<void*, xen_result_func>(sigc::ptr_fun(&rpcWrite), result_handle, result_func));
    httpClient->run();

    return httpClient->getResult();
}


Session::Session(const ConnectSpec& cs)
    : XenObject(XenObject::SESSION, *this, NULL, NULL, NULL)
    , _state(NONE)
    , _connectSpec(cs)
    , _ptr(NULL)
    , _objectStore(new XenObjectStore)
    , _monitoring(false)
{
    Trace trace(this, "Session::ctor(%s,%s,%s)", cs.displayname.c_str(), cs.hostname.c_str(), cs.username.c_str());
}


Session::Session()
    : XenObject(XenObject::SESSION, *this, NULL, NULL, NULL)
    , _state(NONE)
    , _connectSpec()
    , _ptr(NULL)
    , _monitoring(false)
{
    Trace trace(this, "Session::ctor");
}


Session::~Session()
{
    Trace trace(this, "Session::dtor");
    if (_state != NONE)
    {
        disconnect();
    }
}


bool Session::isConnected() const
{
    int state = _state;
    return (state == PRIMARY || state == SECONDARY) ? true : false;
}


bool Session::connect()
{
    Trace trace(this, "Session::connect");
    int state = InterlockedCompareExchange(&_state, PRIMARY_PENDING, NONE);
    if (state != NONE)
    {
        trace.put("return=false (state=%d)", state);
        //emit(CONNECT_FAILED);
        return false;
    }
    _url = Glib::ustring::compose("https://%1", _connectSpec.hostname);
    Glib::ustring pw = _connectSpec.descramblePassword();
    _ptr = xen_session_login_with_password(rpcExecute, this, _connectSpec.username.c_str(), pw.c_str(), xen_api_latest_version);
    pw.clear();
    trace.put("return=%d", _ptr->ok);
    if (_ptr->ok)
    {
        InterlockedExchange(&_state, PRIMARY);
        char* uuid = NULL;
        if (xen_session_get_uuid(_ptr, &uuid, _ptr))
        {
            _uuid = uuid;
            free(uuid);
        }
        else
        {
            Logger::instance().error("xen_session_get_uuid(P) failed.");
            xen_session_clear_error(_ptr);
        }
    }
    else
    {
        emit(CONNECT_FAILED);
    }
    return _ptr->ok;
}


bool Session::connect(const Session& other)
{
    Trace trace(this, "Session::connect(%zx)", &other);
    int state = InterlockedCompareExchange(&_state, SECONDARY_PENDING, NONE);
    if (state != NONE)
    {
        trace.put("return=false (state=%d)", state);
        //emit(CONNECT_FAILED);
        return false;
    }
    _connectSpec = other._connectSpec;
    _url = Glib::ustring::compose("https://%1", _connectSpec.hostname);
    Glib::ustring pw = _connectSpec.descramblePassword();
    _ptr = xen_session_slave_local_login_with_password(rpcExecute, this, _connectSpec.username.c_str(), pw.c_str());
    pw.clear();
    trace.put("return=%d", _ptr->ok);
    if (_ptr->ok)
    {
        InterlockedExchange(&_state, SECONDARY);
        _objectStore = other._objectStore;
#if 0
        // always fails
        char* uuid = NULL;
        if (xen_session_get_uuid(_ptr, &uuid, _ptr))
        {
            _uuid = uuid;
            free(uuid);
        }
        else
        {
            Logger::instance().error("xen_session_get_uuid(S) failed.");
            xen_session_clear_error(_ptr);
        }
#endif
    }
    else
    {
        emit(CONNECT_FAILED);
    }
    return _ptr->ok;
}


bool Session::disconnect()
{
    Trace trace(this, "Session::disconnect");
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
        _uuid.clear();
        retval = true; // only if primary is disconnected successfully.
        break;
    case SECONDARY_PENDING: // This is the case that secondary connect operation failed.
        xen_session_local_logout(_ptr);
        _ptr = NULL;
        break;
    case SECONDARY:
        xen_session_local_logout(_ptr);
        _ptr = NULL;
        _uuid.clear();
        break;
    default:
        break;
    }
    trace.put("return=%d", retval);
    return retval;
}


bool Session::succeeded() const
{
    return _ptr && _ptr->ok ? true : false; 
}


bool Session::failed() const
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


bool Session::hasError() const
{
    return _ptr && !_ptr->ok &&
        _ptr->error_description_count &&
        _ptr->error_description &&
        _ptr->error_description[0] ? true : false;
}


bool Session::hasError(const char* error) const
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


bool Session::operator !=(const Session& rhs) const
{
    return !operator ==(rhs);
}
