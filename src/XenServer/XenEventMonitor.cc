// Copyright (C) 2012-2017 Hideaki Narita


#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <new>
#include <stdexcept>
#include "Base/Atomic.h"
#include "Base/StringBuffer.h"
#include "Controller/Controller.h"
#include "Logger/Trace.h"
#include "Util/Util.h"
#include "Host.h"
#include "Network.h"
#include "PhysicalBlockDevice.h"
#include "PhysicalInterface.h"
#include "Session.h"
#include "StorageRepository.h"
#include "VirtualBlockDevice.h"
#include "VirtualDiskImage.h"
#include "VirtualInterface.h"
#include "VirtualMachine.h"
#include "XenEventMonitor.h"
#include "XenObjectStore.h"
#include "XenObjectTypeMap.h"
#include "XenTask.h"


using namespace hnrt;


XenEventMonitor::XenEventMonitor()
    : _connected(false)
{
    Trace trace("XenEventMonitor::ctor");
}


XenEventMonitor::~XenEventMonitor()
{
    Trace trace("XenEventMonitor::dtor");
}


void XenEventMonitor::run(Session& sessionPrimary)
{
    Trace trace("XenEventMonitor::run");

    RefPtr<Session> pSession = RefPtr<Session>(new Session());

    Session& session = *pSession;

    xen_string_set *classes = NULL;

    try
    {
        if (_connected)
        {
            throw std::runtime_error("XenEventMonitor: Already connected.");
        }

        if (!session.connect(sessionPrimary))
        {
            goto done;
        }

        _connected = true;

        xen_string_set *classes = xen_string_set_alloc(1);
        if (!classes)
        {
            throw std::bad_alloc();
        }

        classes->contents[0] = xen_strdup_("*");
        if (!classes->contents[0])
        {
            throw std::bad_alloc();
        }

        if (!xen_event_register(session, classes))
        {
            goto done;
        }

        while (sessionPrimary.isConnected() && _connected)
        {
            XenPtr<xen_event_record_set> events;

            if (!xen_event_next(session, events.address()))
            {
                goto done;
            }

            if (!sessionPrimary.isConnected())
            {
                break;
            }

            std::vector<Record> cache;

            trace.put("count=%zu", events->size);

            for (size_t i = 0; i < events->size; i++)
            {
                xen_event_record* ev = events->contents[i];

                trace.put("%s %s %s",
                          xen_event_operation_to_string(ev->operation),
                          ev->XEN_CLAZZ,
                          ev->ref);

                XenObject::Type type = XenObjectTypeMap::find(ev->XEN_CLAZZ);
                if (type == XenObject::NONE)
                {
                    continue;
                }

                for (size_t j = 0; ; j++)
                {
                    if (j >= cache.size())
                    {
                        cache.push_back(Record(type, ev));
                        break;
                    }
                    else if (cache[j].type == XenObject::TASK && type != XenObject::TASK)
                    {
                        cache.insert(cache.begin() + j, Record(type, ev));
                        break;
                    }
                    else if (cache[j].type == type && !strcmp(cache[j].ref, ev->ref))
                    {
                        cache[j].operation = ev->operation;
                        break;
                    }
                }
            }

            for (size_t i = 0; i < cache.size(); i++)
            {
                if (!cache[i].process(*this, sessionPrimary, session))
                {
                    session.clearError();
                }
            }
        }
    }
    catch (std::bad_alloc e)
    {
        Logger::instance().error("Out of memory.");
    }
    catch (std::runtime_error e)
    {
        Logger::instance().error("%s", e.what());
    }
    catch (...)
    {
        Logger::instance().error("Unhandled exception caught.");
    }

done:

    if (session.failed())
    {
        if (session.hasError(ERROR_SESSION_INVALID))
        {
            Logger::instance().info("Session %s disconnected.",
                                    session.getConnectSpec().hostname.c_str());
        }
        else if (session.hasError(ERROR_TRANSPORT_FAULT))
        {
            Logger::instance().info("Session %s disconnected by peer.",
                                    session.getConnectSpec().hostname.c_str());
        }
        else
        {
            StringBuffer message;
            XenServer::getError(session, message, " ");
            Logger::instance().info("xen_event_register(%s) failed: %s",
                                    session.getConnectSpec().hostname.c_str(),
                                    message.str());
        }
    }

    if (classes)
    {
        xen_string_set_free(classes);
    }

    _connected = false;
}


XenEventMonitor::Record::Record(int type_, const xen_event_record* ev)
    : id(ev->id)
    , timestamp(ev->timestamp)
    , type(type_)
    , operation(ev->operation)
    , ref(ev->ref)
{
}


bool XenEventMonitor::Record::process(XenEventMonitor& monitor, Session& sessionPrimary, Session& session)
{
    Trace trace("XenEventMonitor::Record::process");

    XenObjectStore& store = session.getStore();

    Session::Lock lock(session);

    if (operation == XEN_EVENT_OPERATION_DEL && type != XenObject::TASK)
    {
        store.remove(ref, static_cast<XenObject::Type>(type));
    }
    else
    {
        switch (type)
        {
        case XenObject::HOST:
        {
            RefPtr<Host> host = store.getHost();
            XenPtr<xen_host_record> record;
            if (xen_host_get_record(session, record.address(), host->getHandle()))
            {
                trace.put("xen_host_get_record succeeded.");
            }
            else
            {
                trace.put("xen_host_get_record failed.");
                return false;
            }
            host->setRecord(record);
            break;
        }

        case XenObject::HOST_METRICS:
        {
            RefPtr<Host> host = store.getHost();
            XenPtr<xen_host_metrics_record> record;
            if (xen_host_metrics_get_record(session, record.address(), ref))
            {
                trace.put("xen_host_metrics_get_record succeeded.");
            }
            else
            {
                trace.put("xen_host_metrics_get_record failed.");
                return false;
            }
            host->setMetricsRecord(record);
            break;
        }

        case XenObject::NETWORK:
        {
            XenPtr<xen_network_record> record;
            if (xen_network_get_record(session, record.address(), ref))
            {
                trace.put("xen_network_get_record succeeded.");
            }
            else
            {
                trace.put("xen_network_get_record failed.");
                return false;
            }
            RefPtr<Network> object = store.getNw(ref);
            if (object)
            {
                object->setRecord(record);
            }
            else
            {
                Network::create(sessionPrimary, ref, record);
            }
            for (int i = 0;; i++)
            {
                RefPtr<VirtualMachine> vm = object->getVm(i);
                if (vm)
                {
                    vm->emit(XenObject::RECORD_UPDATED);
                }
                else
                {
                    break;
                }
            }
            break;
        }

        case XenObject::POOL:
        {
            std::list<RefPtr<StorageRepository> > srList;
            if (store.getList(srList))
            {
                for (std::list<RefPtr<StorageRepository> >::const_iterator i = srList.begin(); i != srList.end(); i++)
                {
                    RefPtr<StorageRepository> sr = *i;
                    XenPtr<xen_sr_record> record;
                    if (xen_sr_get_record(session, record.address(), sr->getHandle()))
                    {
                        trace.put("xen_sr_get_record succeeded.");
                        sr->setRecord(record);
                    }
                    else
                    {
                        trace.put("xen_sr_get_record failed.");
                        session.clearError();
                    }
                }
            }
            break;
        }

        case XenObject::POOL_PATCH:
        {
            RefPtr<Host> host = store.getHost();
            host->updatePatchList();
            host->emit(XenObject::RECORD_UPDATED);
            break;
        }

        case XenObject::PBD:
        {
            XenPtr<xen_pbd_record> record;
            if (xen_pbd_get_record(session, record.address(), ref))
            {
                trace.put("xen_pbd_get_record succeeded.");
            }
            else
            {
                trace.put("xen_pbd_get_record failed.");
                return false;
            }
            RefPtr<PhysicalBlockDevice> pbd = store.getPbd(ref);
            if (pbd)
            {
                pbd->setRecord(record);
            }
            else
            {
                pbd = PhysicalBlockDevice::create(sessionPrimary, ref, record);
            }
            RefPtr<StorageRepository> sr = pbd->getSr();
            if (sr)
            {
                sr->emit(XenObject::RECORD_UPDATED);
            }
            break;
        }

        case XenObject::PIF:
        {
            XenPtr<xen_pif_record> record;
            if (xen_pif_get_record(session, record.address(), ref))
            {
                trace.put("xen_pif_get_record succeeded.");
            }
            else
            {
                trace.put("xen_pif_get_record failed.");
                return false;
            }
            RefPtr<PhysicalInterface> object = store.getPif(ref);
            if (object)
            {
                object->setRecord(record);
            }
            else
            {
                PhysicalInterface::create(sessionPrimary, ref, record);
            }
            break;
        }

        case XenObject::SR:
        {
            XenPtr<xen_sr_record> record;
            if (xen_sr_get_record(session, record.address(), ref))
            {
                trace.put("xen_sr_get_record succeeded.");
            }
            else
            {
                trace.put("xen_sr_get_record failed.");
                return false;
            }
            RefPtr<StorageRepository> sr = store.getSr(ref);
            if (sr)
            {
                sr->setRecord(record);
            }
            else
            {
                StorageRepository::create(sessionPrimary, ref, record);
            }
            break;
        }

        case XenObject::TASK:
        {
            RefPtr<XenTask> task = store.getTask(ref);
            if (!task)
            {
                break;
            }
            if (operation == XEN_EVENT_OPERATION_DEL)
            {
                store.remove(ref, static_cast<XenObject::Type>(type));
            }
            XenPtr<xen_task_record> record;
            if (xen_task_get_record(session, record.address(), ref))
            {
                trace.put("xen_task_get_record succeeded.");
            }
            else
            {
                trace.put("xen_task_get_record failed.");
                return false;
            }
            if (!strcmp(record->name_label, "disconnect"))
            {
                monitor.disconnect();
            }
            xen_task_status_type status = XEN_TASK_STATUS_TYPE_UNDEFINED;
            if (xen_task_get_status(session, &status, ref))
            {
                trace.put("xen_task_get_status succeeded: %s", xen_task_status_type_to_string(status));
            }
            else
            {
                trace.put("xen_task_get_status failed.");
                return false;
            }
            task->setStatus(status);
            switch (status)
            {
            case XEN_TASK_STATUS_TYPE_SUCCESS:
            case XEN_TASK_STATUS_TYPE_FAILURE:
            case XEN_TASK_STATUS_TYPE_CANCELLED:
                switch (status)
                {
                case XEN_TASK_STATUS_TYPE_SUCCESS:
                    task->emit(XenObject::TASK_ON_SUCCESS);
                    break;
                case XEN_TASK_STATUS_TYPE_FAILURE:
                    task->emit(XenObject::TASK_ON_FAILURE);
                    break;
                case XEN_TASK_STATUS_TYPE_CANCELLED:
                    task->emit(XenObject::TASK_ON_CANCELLED);
                    break;
                default:
                    break;
                }
                task->getObject().setBusy(false);
                task->broadcast();
                if (xen_task_destroy(session, ref))
                {
                    trace.put("xen_task_destroy succeeded.");
                    store.remove(ref);
                }
                else
                {
                    trace.put("xen_task_destroy failed.");
                    return false;
                }
                break;
            case XEN_TASK_STATUS_TYPE_PENDING:
            {
                double progress = 0.0;
                if (xen_task_get_progress(session, &progress, ref))
                {
                    trace.put("xen_task_get_progress succeeded: %g", progress);
                    task->setProgress(progress);
                    task->emit(XenObject::TASK_IN_PROGRESS);
                }
                else
                {
                    trace.put("xen_task_get_progress failed.");
                    return false;
                }
                break;
            }
            case XEN_TASK_STATUS_TYPE_CANCELLING:
            case XEN_TASK_STATUS_TYPE_UNDEFINED:
            default:
                break;
            }
            break;
        }

        case XenObject::VBD:
        {
            XenPtr<xen_vbd_record> record;
            if (xen_vbd_get_record(session, record.address(), ref))
            {
                trace.put("xen_vbd_get_record succeeded.");
            }
            else
            {
                trace.put("xen_vbd_get_record failed.");
                return false;
            }
            RefPtr<VirtualBlockDevice> vbd = store.getVbd(ref);
            if (vbd)
            {
                vbd->setRecord(record);
            }
            else
            {
                vbd = VirtualBlockDevice::create(sessionPrimary, ref, record);
            }
            RefPtr<VirtualMachine> vm = vbd->getVm();
            if (vm)
            {
                vm->emit(XenObject::RECORD_UPDATED);
            }
            break;
        }

        case XenObject::VDI:
        {
            XenPtr<xen_vdi_record> record;
            if (xen_vdi_get_record(session, record.address(), ref))
            {
                trace.put("xen_vdi_get_record succeeded.");
            }
            else
            {
                trace.put("xen_vdi_get_record failed.");
                return false;
            }
            RefPtr<VirtualDiskImage> vdi = store.getVdi(ref);
            if (vdi)
            {
                vdi->setRecord(record);
            }
            else
            {
                vdi = VirtualDiskImage::create(sessionPrimary, ref, record);
            }
            RefPtr<StorageRepository> sr = vdi->getSr();
            if (sr)
            {
                sr->emit(XenObject::RECORD_UPDATED);
            }
            for (int i = 0;; i++)
            {
                RefPtr<VirtualMachine> vm = vdi->getVm(i);
                if (vm)
                {
                    vm->emit(XenObject::RECORD_UPDATED);
                }
                else
                {
                    break;
                }
            }
            break;
        }

        case XenObject::VIF:
        {
            XenPtr<xen_vif_record> record;
            if (xen_vif_get_record(session, record.address(), ref))
            {
                trace.put("xen_vif_get_record succeeded.");
            }
            else
            {
                trace.put("xen_vif_get_record failed.");
                return false;
            }
            RefPtr<VirtualInterface> vif = store.getVif(ref);
            if (vif)
            {
                vif->setRecord(record);
            }
            else
            {
                vif = VirtualInterface::create(sessionPrimary, ref, record);
            }
            RefPtr<VirtualMachine> vm = vif->getVm();
            if (vm)
            {
                vm->emit(XenObject::RECORD_UPDATED);
            }
            break;
        }

        case XenObject::VM:
        {
            XenPtr<xen_vm_record> record;
            if (xen_vm_get_record(session, record.address(), ref))
            {
                trace.put("xen_vm_get_record succeeded.");
            }
            else
            {
                trace.put("xen_vm_get_record failed.");
                return false;
            }
            RefPtr<VirtualMachine> vm = store.getVm(ref);
            if (vm)
            {
                vm->setRecord(record);
            }
            else
            {
                vm = VirtualMachine::create(sessionPrimary, ref, record);
            }
            if (record->is_a_snapshot)
            {
                vm = store.getVm(record->snapshot_of);
                if (vm)
                {
                    vm->emit(XenObject::SNAPSHOT_UPDATED);
                }
            }
            break;
        }

        case XenObject::VM_METRICS:
        {
            XenPtr<xen_vm_metrics_record> record;
            if (xen_vm_metrics_get_record(session, record.address(), ref))
            {
                trace.put("xen_vm_metrics_get_record succeeded.");
                RefPtr<VirtualMachine> vm = store.getVmByMetrics(ref);
                if (vm)
                {
                    trace.put("getVmByMetrics: %s", vm->getREFID().c_str());
                    vm->setRecord(record);
                }
            }
            else
            {
                trace.put("xen_vm_metrics_get_record failed.");
                return false;
            }
            break;
        }

        case XenObject::VM_GUEST_METRICS:
        {
            XenPtr<xen_vm_guest_metrics_record> record;
            if (xen_vm_guest_metrics_get_record(session, record.address(), ref))
            {
                trace.put("xen_vm_guest_metrics_get_record succeeded.");
                RefPtr<VirtualMachine> vm = store.getVmByGuestMetrics(ref);
                if (vm)
                {
                    trace.put("getVmByGuestMetrics: %s", vm->getREFID().c_str());
                    vm->setRecord(record);
                }
            }
            else
            {
                trace.put("xen_vm_guest_metrics_get_record failed.");
                return false;
            }
            break;
        }

        default:
            break;
        }
    }

    return true;
}
