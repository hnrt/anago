// Copyright (C) 2012-2017 Hideaki Narita


#define NO_TRACE


#include <curl/curl.h>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Util/UUID.h"
#include "View/View.h"
#include "Session.h"
#include "VirtualMachine.h"
#include "VirtualMachineArchive.h"
#include "VirtualMachineExporter.h"
#include "XenRef.h"


using namespace hnrt;


RefPtr<VirtualMachineExporter> VirtualMachineExporter::create(VirtualMachine& vm)
{
    return RefPtr<VirtualMachineExporter>(new VirtualMachineExporter(vm));
}


VirtualMachineExporter::VirtualMachineExporter(VirtualMachine& vm)
    : XenObject(VM_EXPORTER, vm.getSession(), NULL, UUID::generate().c_str(), "VirtualMachineExporter")
    , _vm(vm)
{
    TRACE(StringBuffer().format("VirtualMachineExporter::ctor(%s)", _vm.getName().c_str()));
    init();
}


VirtualMachineExporter::~VirtualMachineExporter()
{
    TRACE(StringBuffer().format("VirtualMachineExporter::dtor(%s)", _vm.getName().c_str()));
    fini();
}


void VirtualMachineExporter::init()
{
}


void VirtualMachineExporter::fini()
{
}


static size_t receive(void* ptr, size_t size, size_t nmemb, VirtualMachineExporter* pThis)
{
    size_t len = size * nmemb;
    size_t ret = pThis->parse(ptr, len) ? len : 0;
    return ret;
}


void VirtualMachineExporter::run(const char* path, bool verify)
{
    TRACE(StringBuffer().format("VirtualMachineExporter::run(%s)", _vm.getName().c_str()), "path=\"%s\" verify=%d", path, verify);

    XenObject::Busy busy(&_session);

    {
        Glib::Mutex::Lock lock(_mutex);
        _xva = VirtualMachineArchive::create(path, "w", *this);
        _state = VirtualMachineOperationState::EXPORT_INPROGRESS;
        _verified = 0;
        _abort = false;
        _lastUpdated = 0;
    }

    XenRef<xen_task, xen_task_free_t> task;

    CURL* curl = NULL;

    try
    {
        if (!_session.isConnected())
        {
            throw "session";
        }

        char exportString[] = { "export" };
        StringBuffer desc;
        desc.format("export from=%s", _vm.getREFID().c_str());
        if (!xen_task_create(_session, &task, exportString, desc.ptr()))
        {
            throw "xen_task_create";
        }

        if (!_xva->open())
        {
            throw "fopen";
        }

        emit(XenObject::EXPORT_PENDING);

        curl = curl_easy_init();
        if (!curl)
        {
            throw "curl_easy_init";
        }

        StringBuffer url;
        ConnectSpec cs = _session.getConnectSpec();
        url.format("http://%s/export?session_id=%s&task_id=%s&ref=%s",
                   cs.hostname.c_str(), _session->session_id, (const char*)(xen_task)task, _vm.getREFID().c_str());

        TRACEPUT("url=%s", url.str());

        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        //curl_easy_setopt(curl, CURLOPT_STDERR, stderr);
        curl_easy_setopt(curl, CURLOPT_URL, url.str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);

        CURLcode result = curl_easy_perform(curl);

        _xva->close();

        if (_state == VirtualMachineOperationState::EXPORT_CANCELED)
        {
            emit(XenObject::EXPORT_CANCELED);
            goto done;
        }

        if (result != CURLE_OK)
        {
            Logger::instance().warn("CURL: %d (%s)", (int)result, curl_easy_strerror(result));
            throw "curl_easy_perform";
        }

        emit(XenObject::EXPORTED);

        if (verify)
        {
            emit(XenObject::VERIFY_PENDING);
            _state = VirtualMachineOperationState::EXPORT_VERIFY_INPROGRESS;
            if (!_xva->open(NULL, "r"))
            {
                Logger::instance().warn("%s\t%'zu bytes exported, but cannot be opened for reading.", _xva->path(), _xva->nbytes());
                _state = VirtualMachineOperationState::EXPORT_VERIFY_FAILURE;
                emit(XenObject::VERIFY_FAILED);
            }
            else if (!_xva->validate(_verified, (bool&)_abort))
            {
                Logger::instance().warn("%s\t%'zu bytes exported, but checksum mismatched at %d%%.", _xva->path(), _xva->nbytes(), _verified);
                _state = VirtualMachineOperationState::EXPORT_VERIFY_FAILURE;
                emit(XenObject::VERIFY_FAILED);
            }
            else
            {
                Logger::instance().info("%s\t%'zu bytes exported and verified.", _xva->path(), _xva->nbytes());
                _state = VirtualMachineOperationState::EXPORT_VERIFY_SUCCESS;
                emit(XenObject::VERIFIED);
            }
        }
        else
        {
            Logger::instance().info("%s\t%'zu bytes exported.", _xva->path(), _xva->nbytes());
            _state = VirtualMachineOperationState::EXPORT_SUCCESS;
        }
    }
    catch (...)
    {
        if (_state.isActive())
        {
            _state = VirtualMachineOperationState::EXPORT_FAILURE;
            emit(XenObject::EXPORT_FAILED);
        }
    }

done:

    if (curl)
    {
        curl_easy_cleanup(curl);
    }

    if (task)
    {
        xen_task_destroy(_session, task);
    }

    {
        Glib::Mutex::Lock lock(_mutex);
        _xva.reset();
    }
}


bool VirtualMachineExporter::parse(void* ptr, size_t len)
{
    TRACE("VirtualMachineExporter::parse", "ptr=%zx len=%zu", ptr, len);

    if (_abort)
    {
        _state = VirtualMachineOperationState::EXPORT_CANCELED;
        return false;
    }

    if (len)
    {
        if (!_xva->write(ptr, len))
        {
            _state = VirtualMachineOperationState::EXPORT_FAILURE;
            emit(XenObject::EXPORT_FAILED);
            return false;
        }
        TRACEPUT("total=%'zu", _xva->nbytes());
        time_t now = time(NULL);
        if (_lastUpdated < now)
        {
            _lastUpdated = now;
            emit(XenObject::EXPORTING);
        }
    }

    return true;
}


Glib::ustring VirtualMachineExporter::path()
{
    Glib::Mutex::Lock lock(_mutex);
    return _xva ? _xva->path() : Glib::ustring();
}


int64_t VirtualMachineExporter::size()
{
    Glib::Mutex::Lock lock(_mutex);
    return _xva ? _xva->size() : -1;
}


int64_t VirtualMachineExporter::nbytes()
{
    Glib::Mutex::Lock lock(_mutex);
    return _xva ? _xva->nbytes() : -1;
}


int VirtualMachineExporter::percent()
{
    Glib::Mutex::Lock lock(_mutex);
    if (_xva)
    {
        int64_t size = _xva->size();
        int64_t nbytes = _xva->nbytes();
        int percent = -1;
        if (size)
        {
            percent = (int)(((nbytes * 1000) / size + 5) / 10);
        }
        return percent;
    }
    else
    {
        return -1;
    }
}
