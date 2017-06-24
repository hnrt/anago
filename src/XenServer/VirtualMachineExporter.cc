// Copyright (C) 2012-2017 Hideaki Narita


#define NO_TRACE


#include <glibmm.h>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Protocol/HttpClient.h"
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
    TRACEFUN(this, "VirtualMachineExporter::ctor(%s)", vm->getName().c_str());
    _vm = vm;
}


VirtualMachineExporter::~VirtualMachineExporter()
{
    TRACEFUN(this, "VirtualMachineExporter::dtor(%s)", _vm->getName().c_str());
}


void VirtualMachineExporter::run(const char* path, bool verify)
{
    TRACEFUN(this, "VirtualMachineExporter::run(%s,%d)", path, verify);

    XenObject::Busy busy(_session);

    init(path, verify);

    XenRef<xen_task, xen_task_free_t> task;

    try
    {
        if (!_session.isConnected())
        {
            throw StringBuffer().format("Session is disconnected.");
        }

        char exportString[] = { "export" };
        StringBuffer desc;
        desc.format("export from=%s", _vm->getREFID().c_str());
        if (!xen_task_create(_session, &task, exportString, desc.ptr()))
        {
            throw StringBuffer().format("xen_task_create failed.");
        }
        _taskId = (const char*)(xen_task)task;

        if (!_xva->open())
        {
            throw StringBuffer().format("%s: %s", strerror(_xva->error()), _xva->path());
        }

        emit(XenObject::EXPORT_PENDING);

        try
        {
            StringBuffer url;
            ConnectSpec cs = _session.getConnectSpec();
            url.format("http://%s/export?session_id=%s&task_id=%s&ref=%s",
                       cs.hostname.c_str(), _session->session_id, _taskId.c_str(), _vm->getREFID().c_str());

            TRACEPUT("url=%s", url.str());

            RefPtr<HttpClient> httpClient = HttpClient::create();
            httpClient->init();
            httpClient->setUrl(url.str());
            bool result = httpClient->run(*this);

            if (_state != VirtualMachineOperationState::EXPORT_FAILURE &&
                _state != VirtualMachineOperationState::EXPORT_CANCELED)
            {
                if (result)
                {
                    Logger::instance().info("Exported %'zu bytes: %s", _xva->nbytes(), _xva->path());
                    _state = VirtualMachineOperationState::EXPORT_SUCCESS;
                    emit(XenObject::EXPORTED);
                }
                else
                {
                    throw StringBuffer().format("%s", httpClient->getError());
                }
            }
        }
        catch (StringBuffer msg)
        {
            Logger::instance().warn("Export failed: %s", msg.str());
            _state = VirtualMachineOperationState::EXPORT_FAILURE;
            emit(XenObject::EXPORT_FAILED);
        }

        _xva->close();

        if (_verify &&
            _state == VirtualMachineOperationState::EXPORT_SUCCESS)
        {
            try
            {
                _state = VirtualMachineOperationState::EXPORT_VERIFY_PENDING;
                emit(XenObject::VERIFY_PENDING);

                if (!_xva->open(NULL, "r"))
                {
                    throw StringBuffer().format("%s: %s", strerror(_xva->error()), _xva->path());
                }

                _state = VirtualMachineOperationState::EXPORT_VERIFY_INPROGRESS;

                if (_xva->validate(_abort))
                {
                    Logger::instance().info("Verified %'zu bytes: %s", _xva->nbytes(), _xva->path());
                    _state = VirtualMachineOperationState::EXPORT_VERIFY_SUCCESS;
                    emit(XenObject::VERIFIED);
                }
                else if (_xva->error() == ECANCELED)
                {
                    Logger::instance().info("Verify canceled: %s", _xva->path());
                    _state = VirtualMachineOperationState::EXPORT_VERIFY_CANCELED;
                    emit(XenObject::VERIFY_CANCELED);
                }
                else if (_xva->error() == EPROTO)
                {
                    throw StringBuffer().format("Stopped at %'zu: %s", _xva->nbytes(), _xva->path());
                }
                else
                {
                    throw StringBuffer().format("%s: %s", strerror(_xva->error()), _xva->path());
                }
            }
            catch (StringBuffer msg)
            {
                Logger::instance().warn("Verify failed: %s", msg.str());
                _state = VirtualMachineOperationState::EXPORT_VERIFY_FAILURE;
                emit(XenObject::VERIFY_FAILED);
            }
        }
    }
    catch (StringBuffer msg)
    {
        Logger::instance().warn("Export failed: %s", msg.str());
        _state = VirtualMachineOperationState::EXPORT_FAILURE;
        emit(XenObject::EXPORT_FAILED);
    }
    catch (...)
    {
        Logger::instance().warn("Unhandled exception caught.");
        _state = VirtualMachineOperationState::EXPORT_FAILURE;
        emit(XenObject::EXPORT_FAILED);
    }

    if (task)
    {
        xen_task_destroy(_session, task);
    }
}


void VirtualMachineExporter::init(const char* path, bool verify)
{
    Glib::Mutex::Lock lock(_mutex);
    VirtualMachinePorter::init(path, "w", VirtualMachineOperationState::EXPORT_PENDING);
    _vm->setBusy(true);
    _verify = verify;
}


bool VirtualMachineExporter::write(HttpClient& httpClient, const void* ptr, size_t len)
{
    TRACEFUN(this, "VirtualMachineExporter::write(%zx,%zu)", ptr, len);

    if (_abort)
    {
        Logger::instance().info("Export canceled: %s", _xva->path());
        _state = VirtualMachineOperationState::EXPORT_CANCELED;
        emit(XenObject::EXPORT_CANCELED);
        httpClient.cancel();
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
            Logger::instance().warn("Export failed: %s: %s", strerror(_xva->error()), _xva->path());
            _state = VirtualMachineOperationState::EXPORT_FAILURE;
            emit(XenObject::EXPORT_FAILED);
            return false;
        }
    }

    return true;
}
