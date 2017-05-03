// Copyright (C) 2012-2017 Hideaki Narita


#define NO_TRACE


#include <glibmm.h>
#include <curl/curl.h>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Host.h"
#include "Session.h"
#include "VirtualMachine.h"
#include "VirtualMachineArchive.h"
#include "VirtualMachineImporter.h"
#include "XenObjectStore.h"


using namespace hnrt;


RefPtr<VirtualMachineImporter> VirtualMachineImporter::create(Session& session)
{
    return RefPtr<VirtualMachineImporter>(new VirtualMachineImporter(session));
}


VirtualMachineImporter::VirtualMachineImporter(Session& session)
    : VirtualMachinePorter(VM_IMPORTER, session, "VirtualMachineImporter")
{
    TRACE("VirtualMachineImporter::ctor");
}


VirtualMachineImporter::~VirtualMachineImporter()
{
    TRACE("VirtualMachineImporter::dtor");
}


static curlioerr IoControl(CURL *handle, curliocmd cmd, VirtualMachineImporter* pThis)
{
    TRACE(StringBuffer().format("IoControl(%d)", cmd));

    (void)handle;

    switch (cmd)
    {
    case CURLIOCMD_RESTARTREAD:
        pThis->rewind();
        break;

    default:
        return CURLIOE_UNKNOWNCMD;
    }

    return CURLIOE_OK;
}


static size_t SendData(void* ptr, size_t size, size_t nmemb, VirtualMachineImporter* pThis)
{
    size_t len = size * nmemb;
    size_t ret = pThis->read(ptr, len);
    return ret;
}


static size_t ReceiveData(void* ptr, size_t size, size_t nmemb, VirtualMachineImporter* pThis)
{
    size_t len = size * nmemb;
    //fwrite(ptr, 1, len, stdout);
    return len;
}


void VirtualMachineImporter::run(const char* path)
{
    TRACE("VirtualMachineImporter::run", "path=\"%s\"", path);

    XenObject::Busy busy(&_session);

    init(path);

    XenRef<xen_task, xen_task_free_t> task;

    CURL* curl = NULL;
    struct curl_slist *chunk = NULL;

    try
    {
        if (!_session.isConnected())
        {
            throw StringBuffer().format("Session is disconnected.");
        }

        char importString[] = { "import" };
        StringBuffer desc;
        desc.format("import to=%s", _session.getStore().getHost()->getREFID().c_str());
        if (!xen_task_create(_session, &task, importString, desc.ptr()))
        {
            throw StringBuffer().format("xen_task_create failed.");
        }
        _taskId = (const char*)(xen_task)task;

        if (!_xva->open())
        {
            throw StringBuffer().format("%s: %s", strerror(_xva->error()), _xva->path());
        }

        emit(XenObject::IMPORT_PENDING);

        curl = curl_easy_init();
        if (!curl)
        {
            throw StringBuffer().format("CURL initialize failed.");
        }

        StringBuffer url;
        ConnectSpec cs = _session.getConnectSpec();
        url.format("http://%s/import?session_id=%s&task_id=%s",
                   cs.hostname.c_str(), _session->session_id, _taskId.c_str());

        TRACEPUT("url=%s", url.str());

        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        //curl_easy_setopt(curl, CURLOPT_STDERR, stderr);
        curl_easy_setopt(curl, CURLOPT_URL, url.str());
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, _xva->size());
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, SendData);
        curl_easy_setopt(curl, CURLOPT_READDATA, this);
        curl_easy_setopt(curl, CURLOPT_IOCTLFUNCTION, IoControl);
        curl_easy_setopt(curl, CURLOPT_IOCTLDATA, this);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ReceiveData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);

        chunk = curl_slist_append(chunk, "Expect:");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        CURLcode result = curl_easy_perform(curl);

        if (_state != VirtualMachineOperationState::IMPORT_FAILURE &&
            _state != VirtualMachineOperationState::IMPORT_CANCELED)
        {
            if (result == CURLE_OK)
            {
            success:
                Logger::instance().info("Imported %'zu bytes: %s", _xva->nbytes(), _xva->path());
                _state = VirtualMachineOperationState::IMPORT_SUCCESS;
                emit(XenObject::IMPORTED);
            }
            else
            {
                if (result == CURLE_RECV_ERROR)
                {
                    long response = -1;
                    if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response) == CURLE_OK)
                    {
                        if (response == 200 && _xva->size() == _xva->nbytes())
                        {
                            goto success;
                        }
                    }
                }
                throw StringBuffer().format("CURL: %d (%s)", (int)result, curl_easy_strerror(result));
            }
        }
    }
    catch (StringBuffer msg)
    {
        Logger::instance().warn("Import failed: %s", msg.str());
        _state = VirtualMachineOperationState::IMPORT_FAILURE;
        emit(XenObject::IMPORT_FAILED);
    }
    catch (...)
    {
        Logger::instance().warn("Unhandled exception caught.");
        _state = VirtualMachineOperationState::IMPORT_FAILURE;
        emit(XenObject::IMPORT_FAILED);
    }

    if (chunk)
    {
        curl_slist_free_all(chunk);
    }

    if (curl)
    {
        curl_easy_cleanup(curl);
    }

    if (task)
    {
        xen_task_destroy(_session, task);
    }
}


void VirtualMachineImporter::init(const char* path)
{
    Glib::Mutex::Lock lock(_mutex);
    VirtualMachinePorter::init(path, "r", VirtualMachineOperationState::IMPORT_PENDING);
    _vm.reset();
}


size_t VirtualMachineImporter::read(void* ptr, size_t len)
{
    TRACE("VirtualMachineImporter::read", "ptr=%zx len=%zu", ptr, len);

    if (_abort)
    {
        Logger::instance().info("Import canceled: %s", _xva->path());
        _state = VirtualMachineOperationState::IMPORT_CANCELED;
        emit(XenObject::IMPORT_CANCELED);
        return CURL_READFUNC_ABORT;
    }

    size_t ret = _xva->read(ptr, len);
    if (ret)
    {
        TRACEPUT("read=%'zu total=%'zu", ret, _xva->nbytes());
        _state = VirtualMachineOperationState::IMPORT_INPROGRESS;
        time_t now = time(NULL);
        if (_lastUpdated < now)
        {
            _lastUpdated = now;
            checkVm();
            emit(XenObject::IMPORTING);
        }
    }
    else if (_xva->error())
    {
        Logger::instance().warn("Import failed: %s: %s", strerror(_xva->error()), _xva->path());
        _state = VirtualMachineOperationState::IMPORT_FAILURE;
        emit(XenObject::IMPORT_FAILED);
        return CURL_READFUNC_ABORT;
    }
    else
    {
        TRACEPUT("read=%'zu total=%'zu", ret, _xva->nbytes());
    }

    return ret;
}


void VirtualMachineImporter::rewind()
{
    _xva->rewind();
}
