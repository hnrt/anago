// Copyright (C) 2012-2017 Hideaki Narita


#define NO_TRACE


#include <glibmm.h>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Protocol/HttpClient.h"
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
    TRACEFUN(this, "VirtualMachineImporter::ctor");
}


VirtualMachineImporter::~VirtualMachineImporter()
{
    TRACEFUN(this, "VirtualMachineImporter::dtor");
}


void VirtualMachineImporter::run(const char* path)
{
    TRACEFUN(this, "VirtualMachineImporter::run(%s)", path);

    XenObject::Busy busy(_session);

    init(path);

    XenRef<xen_task, xen_task_free_t> task;

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

        StringBuffer url;
        ConnectSpec cs = _session.getConnectSpec();
        url.format("http://%s/import?session_id=%s&task_id=%s",
                   cs.hostname.c_str(), _session->session_id, _taskId.c_str());

        TRACEPUT("url=%s", url.str());

        RefPtr<HttpClient> httpClient = HttpClient::create();
        httpClient->init();
        httpClient->setUrl(url.str());
        httpClient->setUpload(_xva->size());
        httpClient->removeExpectHeader();
        httpClient->setReadFunction(sigc::bind<HttpClient*>(sigc::mem_fun(*this, &VirtualMachineImporter::read), httpClient));
        httpClient->setRewindFunction(sigc::mem_fun(*this, &VirtualMachineImporter::rewind));
        bool result = httpClient->run();

        if (_state != VirtualMachineOperationState::IMPORT_FAILURE &&
            _state != VirtualMachineOperationState::IMPORT_CANCELED)
        {
            if (result || (httpClient->getStatus() == 200 && _xva->size() == _xva->nbytes()))
            {
                Logger::instance().info("Imported %'zu bytes: %s", _xva->nbytes(), _xva->path());
                _state = VirtualMachineOperationState::IMPORT_SUCCESS;
                emit(XenObject::IMPORTED);
            }
            else
            {
                throw StringBuffer().format("%s", httpClient->getError());
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


size_t VirtualMachineImporter::read(void* ptr, size_t len, HttpClient* httpClient)
{
    TRACEFUN(this, "VirtualMachineImporter::read(%zu)", len);

    if (_abort)
    {
        Logger::instance().info("Import canceled: %s", _xva->path());
        _state = VirtualMachineOperationState::IMPORT_CANCELED;
        emit(XenObject::IMPORT_CANCELED);
        httpClient->cancel();
        return 0;
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
        return 0;
    }
    else
    {
        TRACEPUT("read=%'zu total=%'zu", ret, _xva->nbytes());
    }

    return ret;
}


void VirtualMachineImporter::rewind()
{
    TRACEFUN(this, "VirtualMachineImporter::rewind");

    _xva->rewind();
}
