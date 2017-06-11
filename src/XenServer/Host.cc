// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Base/Atomic.h"
#include "Base/StringBuffer.h"
#include "Controller/Controller.h"
#include "File/File.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Model/PatchBase.h"
#include "Model/PatchRecord.h"
#include "Host.h"
#include "Session.h"
#include "VirtualDiskImage.h"
#include "XenObjectStore.h"
#include "XenRef.h"


using namespace hnrt;


RefPtr<Host> Host::create(const ConnectSpec& cs)
{
    RefPtr<Session> session = RefPtr<Session>(new Session(cs));
    RefPtr<Host> host(new Host(*session));
    session->getStore().setHost(host);
    return host;
}


Host::Host(Session& session)
    : XenObject(XenObject::HOST, session, NULL, NULL, NULL)
    , _state(STATE_NONE)
{
    Trace trace(StringBuffer().format("Host@%zx::ctor", this));
}


Host:: ~Host()
{
    Trace trace(StringBuffer().format("Host@%zx::dtor", this));
}


int Host::setBusy(bool value)
{
    TRACE(StringBuffer().format("Host@%zx::setBusy", this));
    int count = XenObject::setBusy(value);
    TRACEPUT("count=%d state=%d", count, _state);
    if (!count)
    {
        switch (_state)
        {
        case STATE_CONNECTED:
            setDisplayStatus(gettext("Connected"));
            break;
        case STATE_DISCONNECTED:
            setDisplayStatus(gettext("Disconnected"));
            break;
        case STATE_DISCONNECTED_BY_PEER:
            setDisplayStatus(gettext("Disconnected by peer"));
            _state = STATE_DISCONNECTED;
            break;
        case STATE_CONNECT_FAILED:
            setDisplayStatus(gettext("Failed to connect"));
            break;
        case STATE_DISCONNECT_PENDING:
        case STATE_SHUTDOWN:
        case STATE_SHUTDOWN_FAILED:
        case STATE_REBOOTED:
        case STATE_REBOOT_FAILED:
            break;
        case STATE_SHUTDOWN_PENDING:
            shutdown();
            break;
        case STATE_REBOOT_PENDING:
            reboot();
            break;
        default:
            break;
        }
    }
    return count;
}


XenPtr<xen_host_record> Host::getRecord() const
{
    Glib::Mutex::Lock lock(const_cast<Host*>(this)->_mutex);
    return _record;
}


void Host::setRecord(const XenPtr<xen_host_record>& record)
{
    if (record)
    {
        Glib::Mutex::Lock lock(_mutex);
        _record = record;
        if (record->name_label)
        {
            _name = record->name_label;
        }
    }
    else
    {
        return;
    }
    emit(RECORD_UPDATED);
}


XenPtr<xen_host_metrics_record> Host::getMetricsRecord() const
{
    Glib::Mutex::Lock lock(const_cast<Host*>(this)->_mutex);
    return _metricsRecord;
}


void Host::setMetricsRecord(const XenPtr<xen_host_metrics_record>& record)
{
    if (record)
    {
        Glib::Mutex::Lock lock(_mutex);
        _metricsRecord = record;
    }
    else
    {
        return;
    }
    emit(RECORD_UPDATED);
}


void Host::onConnectPending()
{
    _state = STATE_CONNECT_PENDING;
    setDisplayStatus(gettext("Connecting..."));
}


bool Host::onConnected()
{
    _state = STATE_CONNECTED;
    _session.getConnectSpec().lastAccess = (long)time(NULL);
    XenRef<xen_host, xen_host_free_t> host;
    if (!xen_session_get_this_host(_session, &host, _session))
    {
        Logger::instance().warn("Host::connect: xen_session_get_this_host failed.");
        emit(ERROR);
        return false;
    }
    _refid = host.toString();
    _handle = const_cast<void*>(reinterpret_cast<const void*>(_refid.c_str()));
    XenPtr<xen_host_record> record;
    if (xen_host_get_record(_session, record.address(), host))
    {
        Glib::Mutex::Lock lock(_mutex);
        _record = record;
        _uuid = record->uuid ? record->uuid : "";
        _name = record->name_label ? record->name_label : "";
        if (!_uuid.empty())
        {
            _session.getConnectSpec().uuid = _uuid;
        }
    }
    else
    {
        Logger::instance().warn("Host::connect: xen_host_get_recrod failed.");
        emit(ERROR);
        return false;
    }
    XenRef<xen_host_metrics, xen_host_metrics_free_t> metrics;
    if (xen_host_get_metrics(_session, &metrics, host))
    {
        XenPtr<xen_host_metrics_record> metricsRecord;
        if (xen_host_metrics_get_record(_session, metricsRecord.address(), metrics))
        {
            Glib::Mutex::Lock lock(_mutex);
            _metricsRecord = metricsRecord;
        }
        else
        {
            Logger::instance().warn("Host::connect: xen_host_metrics_get_record failed.");
            emit(ERROR);
            return false;
        }
    }
    else
    {
        Logger::instance().warn("Host::connect: xen_host_get_metrics failed.");
        emit(ERROR);
        return false;
    }
    initPatchList();
    updatePatchList();
    emit(CONNECTED);
    return true;
}


void Host::onConnectFailed()
{
    _state = STATE_CONNECT_FAILED;
}


void Host::onDisconnectPending()
{
    if (_state == STATE_CONNECTED)
    {
        _state = STATE_DISCONNECT_PENDING;
        setDisplayStatus(gettext("Disconnecting..."));
    }
}


void Host::onDisconnected()
{
    emit(DISCONNECTED);
    if (_state == STATE_DISCONNECT_PENDING)
    {
        _state = STATE_DISCONNECTED;
    }
    _patchList.clear();
}


void Host::onDisconnectedByPeer()
{
    if (_state == STATE_CONNECTED)
    {
        _state = STATE_DISCONNECTED_BY_PEER;
    }
    onDisconnected();
}


bool Host::shutdown()
{
    if (_busyCount)
    {
        if (_state == STATE_CONNECTED)
        {
            _state = STATE_SHUTDOWN_PENDING;
        }
        return true;
    }
    if (_session)
    {
        if (xen_host_disable(_session, _handle) &&
            xen_host_shutdown(_session, _handle))
        {
            setDisplayStatus(gettext("Shut down successfully"));
            _state = STATE_SHUTDOWN;
            return true;
        }
        else
        {
            setDisplayStatus(gettext("Failed to shut down"));
            _state = STATE_SHUTDOWN_FAILED;
            emit(ERROR);
            return false;
        }
    }
    else
    {
        return true;
    }
}


bool Host::reboot()
{
    if (_busyCount)
    {
        if (_state == STATE_CONNECTED)
        {
            _state = STATE_REBOOT_PENDING;
        }
        return true;
    }
    if (_session)
    {
        if (xen_host_disable(_session, _handle) &&
            xen_host_reboot(_session, _handle))
        {
            setDisplayStatus(gettext("Rebooted successfully"));
            _state = STATE_REBOOTED;
            return true;
        }
        else
        {
            setDisplayStatus(gettext("Failed to reboot"));
            _state = STATE_REBOOT_FAILED;
            emit(ERROR);
            return false;
        }
    }
    else
    {
        return true;
    }
}


bool Host::setName(const char* label, const char* description)
{
    if (!xen_host_set_name_label(_session, _handle, (char*)label))
    {
        emit(ERROR);
        return false;
    }

    if (!xen_host_set_name_description(_session, _handle, (char*)description))
    {
        emit(ERROR);
        return false;
    }

    return true;
}


void Host::initPatchList()
{
    TRACE(StringBuffer().format("Host@%zx::initPatchList", this));
    XenPtr<xen_host_record> record = getRecord();
    if (!record)
    {
        TRACEPUT("No record.");
        return;
    }
    const char* version = XenServer::find(record->software_version, "product_version");
    if (!version)
    {
        TRACEPUT("No version.");
        return;
    }
    RefPtr<PatchBase> pb = Model::instance().getPatchBase();
    PatchBase::RecordIterator iter = pb->getRecordIterator(version);
    for (RefPtr<PatchRecord> patchRecord = iter.next(); patchRecord; patchRecord = iter.next())
    {
        TRACEPUT("PatchRecord={%s}", patchRecord->label.c_str());
        for (std::list<RefPtr<PatchRecord> >::iterator i = _patchList.begin();; i++)
        {
            if (i == _patchList.end())
            {
                RefPtr<PatchRecord> patchRecord2 = PatchRecord::create();
                *patchRecord2 = *patchRecord;
                _patchList.push_back(patchRecord2);
                break;
            }
            else if (strcasecmp((*i)->label.c_str(), patchRecord->label.c_str()) > 0)
            {
                RefPtr<PatchRecord> patchRecord2 = PatchRecord::create();
                *patchRecord2 = *patchRecord;
                _patchList.insert(i, patchRecord2);
                break;
            }
        }
    }
}

#if 0
static bool IsApplied(const char* refid, XenPtr<xen_host_patch_set> hostPatchSet)
{
    if (hostPatchSet && hostPatchSet->size)
    {
        for (size_t i = 0; i < hostPatchSet->size; i++)
        {
            if (!strcmp(refid, (const char*)hostPatchSet->contents[i]))
            {
                return true;
            }
        }
    }
    return false;
}


static bool IsApplied(XenPtr<xen_pool_patch_record> patchRecord, XenPtr<xen_host_patch_set> hostPatchSet)
{
    if (patchRecord->host_patches && patchRecord->host_patches->size)
    {
        for (size_t i = 0; i < patchRecord->host_patches->size; i++)
        {
            if (!patchRecord->host_patches->contents[i]->is_record && IsApplied((const char*)patchRecord->host_patches->contents[i]->u.handle, hostPatchSet))
            {
                return true;
            }
        }
    }
    return false;
}


static void UpdateState(std::list<RefPtr<PatchRecord> >& patchList, XenPtr<xen_pool_patch_record> patchRecord, XenPtr<xen_host_patch_set> hostPatchSet)
{
    PatchState state;
    if (patchRecord->pool_applied)
    {
        state = PatchState::APPLIED;
    }
    else if (IsApplied(patchRecord, hostPatchSet))
    {
        state = PatchState::APPLIED;
    }
    else
    {
        state = PatchState::UPLOADED;
    }
    for (std::list<RefPtr<PatchRecord> >::iterator i = patchList.begin(); i != patchList.end(); i++)
    {
        if ((*i)->uuid == patchRecord->uuid)
        {
            switch ((*i)->state)
            {
            case PatchState::AVAILABLE:
            case PatchState::DOWNLOAD_FAILURE:
            case PatchState::DOWNLOADED:
            case PatchState::UPLOAD_FAILURE:
            case PatchState::UPLOADED:
            case PatchState::APPLIED:
            {
                (*i)->state = state;
                break;
            }
            default:
                break;
            }
            break;
        }
    }
}
#endif

void Host::updatePatchList()
{
    TRACE(StringBuffer().format("Host@%zx::updatePatchList", this));
    for (std::list<RefPtr<PatchRecord> >::iterator i = _patchList.begin(); i != _patchList.end(); i++)
    {
        switch ((*i)->state)
        {
        case PatchState::AVAILABLE:
        case PatchState::DOWNLOADED:
        {
            RefPtr<File> file = (*i)->getFile();
            (*i)->state = file && file->size() ? PatchState::DOWNLOADED : PatchState::AVAILABLE;
            break;
        }
        default:
            break;
        }
    }
    XenPtr<xen_pool_update_set> poolUpdateSet;
    if (xen_pool_update_get_all(_session, poolUpdateSet.address()))
    {
        if (poolUpdateSet)
        {
            TRACEPUT("pool-update: size=%zu", poolUpdateSet->size);
            for (size_t i = 0; i < poolUpdateSet->size; i++)
            {
                if (!poolUpdateSet->contents[i])
                {
                    continue;
                }
                XenPtr<xen_pool_update_record> poolUpdateRecord;
                if (!xen_pool_update_get_record(_session, poolUpdateRecord.address(), poolUpdateSet->contents[i]))
                {
                    TRACEPUT("pool-update: Unavailable: %s", (char*)poolUpdateSet->contents[i]);
                    _session.clearError();
                    continue;
                }
                TRACEPUT("pool-update: uuid=%s", poolUpdateRecord->uuid);
                for (std::list<RefPtr<PatchRecord> >::iterator j = _patchList.begin(); j != _patchList.end(); j++)
                {
                    if ((*j)->uuid == poolUpdateRecord->uuid)
                    {
                        if (poolUpdateRecord->hosts)
                        {
                            TRACEPUT("hosts: size=%zu", poolUpdateRecord->hosts->size);
                            for (size_t k = 0; k < poolUpdateRecord->hosts->size; k++)
                            {
                                if (poolUpdateRecord->hosts->contents[k])
                                {
                                    if (poolUpdateRecord->hosts->contents[k]->is_record)
                                    {
                                        TRACEPUT("xen_host_record_opt: is_record=true not supported.");
                                    }
                                    else if (poolUpdateRecord->hosts->contents[k]->u.handle)
                                    {
                                        TRACEPUT("hosts: %s", (char*)poolUpdateRecord->hosts->contents[k]->u.handle);
                                        if (_refid == (char*)poolUpdateRecord->hosts->contents[k]->u.handle)
                                        {
                                            TRACEPUT("Matched.");
                                            (*j)->state = PatchState::APPLIED;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        TRACEPUT("xen_host_record_opt: handle=null");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        _session.clearError();
    }
    XenPtr<xen_pool_patch_set> poolPatchSet;
    if (xen_pool_patch_get_all(_session, poolPatchSet.address()))
    {
        if (poolPatchSet)
        {
            TRACEPUT("pool-patch: size=%zu", poolPatchSet->size);
            for (size_t i = 0; i < poolPatchSet->size; i++)
            {
                if (!poolPatchSet->contents[i])
                {
                    continue;
                }
                XenPtr<xen_pool_patch_record> poolPatchRecord;
                if (!xen_pool_patch_get_record(_session, poolPatchRecord.address(), poolPatchSet->contents[i]))
                {
                    TRACEPUT("pool-patch: Unavailable: %s", (char*)poolPatchSet->contents[i]);
                    _session.clearError();
                    continue;
                }
                TRACEPUT("pool-patch: uuid=%s size=%zu", poolPatchRecord->uuid, poolPatchRecord->size);
                for (std::list<RefPtr<PatchRecord> >::iterator j = _patchList.begin(); j != _patchList.end(); j++)
                {
                    if ((*j)->uuid == poolPatchRecord->uuid)
                    {
                        if (poolPatchRecord->size)
                        {
                            (*j)->state = PatchState::UPLOADED;
                            (*j)->size = poolPatchRecord->size;
                        }
                        else
                        {
                            (*j)->size = 0;
                        }
                        break;
                    }
                }
            }
        }
    }
    else
    {
        _session.clearError();
    }
    XenPtr<xen_host_patch_set> hostPatchSet;
    if (xen_host_patch_get_all(_session, hostPatchSet.address()))
    {
        if (hostPatchSet)
        {
            for (size_t i = 0; i < hostPatchSet->size; i++)
            {
                if (!hostPatchSet->contents[i])
                {
                    continue;
                }
                XenPtr<xen_host_patch_record> hostPatchRecord;
                if (!xen_host_patch_get_record(_session, hostPatchRecord.address(), hostPatchSet->contents[i]))
                {
                    _session.clearError();
                    continue;
                }
                XenPtr<xen_pool_patch_record> poolPatchRecord;
                if (!xen_pool_patch_get_record(_session, poolPatchRecord.address(), hostPatchRecord->pool_patch->u.handle))
                {
                    _session.clearError();
                    continue;
                }
                TRACEPUT("host-patch: uuid=%s size=%zu", poolPatchRecord->uuid, poolPatchRecord->size);
                for (std::list<RefPtr<PatchRecord> >::iterator j = _patchList.begin(); j != _patchList.end(); j++)
                {
                    if ((*j)->uuid == poolPatchRecord->uuid)
                    {
                        (*j)->state = PatchState::APPLIED;
                        break;
                    }
                }
            }
        }
    }
    else
    {
        _session.clearError();
    }
}


int Host::getPatchList(std::list<RefPtr<PatchRecord> >& list) const
{
    int count = 0;
    for (std::list<RefPtr<PatchRecord> >::const_iterator i = _patchList.begin(); i != _patchList.end(); i++)
    {
        list.push_back(*i);
        count++;
    }
    return count;    
}


RefPtr<PatchRecord> Host::getPatchRecord(const Glib::ustring& uuid) const
{
    RefPtr<PatchRecord> patchRecord;
    for (std::list<RefPtr<PatchRecord> >::const_iterator i = _patchList.begin(); i != _patchList.end(); i++)
    {
        if ((*i)->uuid == uuid)
        {
            patchRecord = *i;
            break;
        }
    }
    return patchRecord;
}


bool Host::applyPatch(const Glib::ustring& uuid)
{
    XenRef<xen_pool_update, xen_pool_update_free_t> updateHandle;
    if (xen_pool_update_get_by_uuid(_session, &updateHandle, const_cast<char*>(uuid.c_str())))
    {
        TRACE1("Host@%zx::applyPatch: update-handle=%s", this, updateHandle.toString().c_str());
        if (xen_pool_update_apply(_session, updateHandle, _handle))
        {
            return true;
        }
        else
        {
            emit(ERROR);
            return false;
        }
    }
    else
    {
        _session.clearError();
    }
    XenRef<xen_pool_patch, xen_pool_patch_free_t> patchHandle;
    if (xen_pool_patch_get_by_uuid(_session, &patchHandle, const_cast<char*>(uuid.c_str())))
    {
        TRACE1("Host@%zx::applyPatch: update-handle=%s", this, patchHandle.toString().c_str());
        char* result = NULL;
        if (xen_pool_patch_apply(_session, &result, patchHandle, _handle))
        {
            free(result);
            return true;
        }
        else
        {
            emit(ERROR);
            return false;
        }
    }
    else
    {
        emit(ERROR);
        return false;
    }
}


bool Host::cleanPatch(const Glib::ustring& uuid)
{
    XenRef<xen_pool_update, xen_pool_update_free_t> updateHandle;
    if (xen_pool_update_get_by_uuid(_session, &updateHandle, const_cast<char*>(uuid.c_str())))
    {
        if (xen_pool_update_pool_clean(_session, updateHandle))
        {
            return true;
        }
        else
        {
            emit(ERROR);
            return false;
        }
    }
    else
    {
        _session.clearError();
    }
    XenRef<xen_pool_patch, xen_pool_patch_free_t> patchHandle;
    if (xen_pool_patch_get_by_uuid(_session, &patchHandle, const_cast<char*>(uuid.c_str())))
    {
        if (xen_pool_patch_pool_clean(_session, patchHandle))
        {
            return true;
        }
        else
        {
            emit(ERROR);
            return false;
        }
    }
    else
    {
        emit(ERROR);
        return false;
    }
}
