// Copyright (C) 2012-2017 Hideaki Narita


//#define NO_TRACE


#include <glibmm.h>
#include <curl/curl.h>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Session.h"
#include "VirtualMachine.h"
#include "VirtualMachineArchive.h"
#include "VirtualMachineExporter.h"


using namespace hnrt;


RefPtr<VirtualMachineExporter> VirtualMachineExporter::create(RefPtr<VirtualMachine> vm)
{
    return RefPtr<VirtualMachineExporter>(new VirtualMachineExporter(vm));
}


VirtualMachineExporter::VirtualMachineExporter(RefPtr<VirtualMachine> vm)
    : VirtualMachinePorter(VM_EXPORTER, vm->getSession(), "VirtualMachineExporter")
{
    TRACE("VirtualMachineExporter::ctor", "vm=\"%s\"", vm->getName().c_str());
    _vm = vm;
}


VirtualMachineExporter::~VirtualMachineExporter()
{
    TRACE("VirtualMachineExporter::dtor");
}


static size_t receive(void* ptr, size_t size, size_t nmemb, VirtualMachineExporter* pThis)
{
    size_t len = size * nmemb;
    size_t ret = pThis->parse(ptr, len) ? len : 0;
    return ret;
}


void VirtualMachineExporter::run(const char* path, bool verify)
{
    TRACE(StringBuffer().format("VirtualMachineExporter::run(%s)", _vm->getName().c_str()), "path=\"%s\" verify=%d", path, verify);

    XenObject::Busy busy(&_session);

    open(path, verify);

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
        desc.format("export from=%s", _vm->getREFID().c_str());
        if (!xen_task_create(_session, &task, exportString, desc.ptr()))
        {
            throw "xen_task_create";
        }
        _taskId = (const char*)(xen_task)task;

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
                   cs.hostname.c_str(), _session->session_id, _taskId.c_str(), _vm->getREFID().c_str());

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

        if (_state != VirtualMachineOperationState::EXPORT_INPROGRESS)
        {
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
                Logger::instance().warn("%s: %'zu bytes exported, but cannot be opened for reading.", _xva->path(), _xva->nbytes());
                _state = VirtualMachineOperationState::EXPORT_VERIFY_FAILURE;
                emit(XenObject::VERIFY_FAILED);
            }
            else if (!_xva->validate(_abort))
            {
                Logger::instance().warn("%s: %'zu bytes exported, but checksum mismatched at %'zu bytes.", _xva->path(), _xva->size(), _xva->nbytes());
                _state = VirtualMachineOperationState::EXPORT_VERIFY_FAILURE;
                emit(XenObject::VERIFY_FAILED);
            }
            else
            {
                Logger::instance().info("%s: %'zu bytes exported and verified.", _xva->path(), _xva->nbytes());
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

    close();
}


void VirtualMachineExporter::open(const char* path, bool verify)
{
    Glib::Mutex::Lock lock(_mutex);
    VirtualMachinePorter::open(path, "w", VirtualMachineOperationState::EXPORT_PENDING);
    _vm->setBusy(true);
    _verify = verify;
}


void VirtualMachineExporter::close()
{
    Glib::Mutex::Lock lock(_mutex);
    VirtualMachinePorter::close();
}


bool VirtualMachineExporter::parse(void* ptr, size_t len)
{
    TRACE("VirtualMachineExporter::parse", "ptr=%zx len=%zu", ptr, len);

    if (_abort)
    {
        _state = VirtualMachineOperationState::EXPORT_CANCELED;
        emit(XenObject::EXPORT_CANCELED);
        return false;
    }

    if (len)
    {
        if (_xva->write(ptr, len))
        {
            TRACEPUT("total=%'zu", _xva->nbytes());
            _state = VirtualMachineOperationState::EXPORT_INPROGRESS;
            time_t now = time(NULL);
            if (_lastUpdated < now)
            {
                _lastUpdated = now;
                emit(XenObject::EXPORTING);
            }
        }
        else
        {
            TRACEPUT("write failed.");
            _state = VirtualMachineOperationState::EXPORT_FAILURE;
            emit(XenObject::EXPORT_FAILED);
            return false;
        }
    }

    return true;
}
