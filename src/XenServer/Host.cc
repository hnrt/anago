// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Base/Atomic.h"
#include "Base/StringBuffer.h"
#include "Controller/Controller.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Model/PatchBase.h"
#include "Model/PatchRecord.h"
#include "Host.h"
#include "Session.h"
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
    int count = XenObject::setBusy(value);
    if (!count)
    {
        switch (_state)
        {
        case STATE_CONNECT_FAILED:
        case STATE_DISCONNECT_PENDING:
        case STATE_DISCONNECTED_BY_PEER:
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


XenPtr<xen_host_record> Host::getRecord()
{
    Glib::Mutex::Lock k(_mutex);
    return _record;
}


void Host::setRecord(const XenPtr<xen_host_record>& record)
{
    if (record)
    {
        Glib::Mutex::Lock k(_mutex);
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


XenPtr<xen_host_metrics_record> Host::getMetricsRecord()
{
    Glib::Mutex::Lock k(_mutex);
    return _metricsRecord;
}


void Host::setMetricsRecord(const XenPtr<xen_host_metrics_record>& record)
{
    if (record)
    {
        Glib::Mutex::Lock k(_mutex);
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
    setDisplayStatus(gettext("Connected"));
    XenRef<xen_host, xen_host_free_t> host;
    if (!xen_session_get_this_host(_session, &host, _session))
    {
        Logger::instance().warn("Host::connect: xen_session_get_this_host failed.");
        emit(ERROR);
        return false;
    }
    _refid = host.toString();
    XenPtr<xen_host_record> record;
    if (xen_host_get_record(_session, record.address(), host))
    {
        Glib::Mutex::Lock k(_mutex);
        _record = record;
        _uuid = record->uuid ? record->uuid : "";
        _name = record->name_label ? record->name_label : "";
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
            Glib::Mutex::Lock k(_mutex);
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
    setDisplayStatus(gettext("Failed to connect"));
}


void Host::onDisconnected()
{
    emit(DISCONNECTED);
    if (_state == STATE_CONNECTED || _state == STATE_DISCONNECT_PENDING)
    {
        setDisplayStatus(gettext("Disconnected"));
    }
    else if (_state == STATE_DISCONNECTED_BY_PEER)
    {
        setDisplayStatus(gettext("Disconnected by peer"));
    }
    _patchList.clear();
    _state = STATE_DISCONNECTED;
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
        if (xen_host_disable(_session, getXenRef()) &&
            xen_host_shutdown(_session, getXenRef()))
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
        if (xen_host_disable(_session, getXenRef()) &&
            xen_host_reboot(_session, getXenRef()))
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
    if (!xen_host_set_name_label(_session, getXenRef(), (char*)label))
    {
        emit(ERROR);
        return false;
    }

    if (!xen_host_set_name_description(_session, getXenRef(), (char*)description))
    {
        emit(ERROR);
        return false;
    }

    return true;
}


void Host::initPatchList()
{
    XenPtr<xen_host_record> record = getRecord();
    if (!record)
    {
        return;
    }
    const char* version = XenServer::find(record->software_version, "product_version");
    if (!version)
    {
        return;
    }
    RefPtr<PatchBase> pb = Model::instance().getPatchBase();
    PatchBase::RecordIterator iter = pb->getRecordIterator(version);
    for (RefPtr<PatchRecord> patchRecord = iter.next(); patchRecord; patchRecord = iter.next())
    {
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
    for (std::list<RefPtr<PatchRecord> >::iterator i = _patchList.begin(); i != _patchList.end(); i++)
    {
        switch ((*i)->state)
        {
        case PatchState::AVAILABLE:
        case PatchState::DOWNLOADED:
        {
            Glib::ustring filename = Glib::ustring::compose("%1.xsupdate", (*i)->label);
            Glib::ustring path = Glib::ustring::compose("%1%2", Model::instance().getAppDir(), filename);
            struct stat sd = { 0 };
            int rc = stat(path.c_str(), &sd);
            (*i)->state = !rc && sd.st_size ? PatchState::DOWNLOADED : PatchState::AVAILABLE;
            break;
        }
        default:
            break;
        }
    }
    XenPtr<xen_pool_patch_set> poolPatchSet;
    if (xen_pool_patch_get_all(_session, poolPatchSet.address()))
    {
        if (poolPatchSet)
        {
            for (size_t i = 0; i < poolPatchSet->size; i++)
            {
                if (!poolPatchSet->contents[i])
                {
                    continue;
                }
                XenPtr<xen_pool_patch_record> poolPatchRecord;
                if (!xen_pool_patch_get_record(_session, poolPatchRecord.address(), poolPatchSet->contents[i]))
                {
                    _session.clearError();
                    continue;
                }
                //g_print("#Host::updatePatchList: poolPatch[%zu] uuid(%s) label(%s) size(%zu)\n", i, poolPatchRecord->uuid, poolPatchRecord->name_label, poolPatchRecord->size);
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
                //g_print("#Host::updatePatchList: hostPatch[%zu] uuid(%s) label(%s) size(%zu)\n", i, hostPatchRecord->uuid, poolPatchRecord->name_label, hostPatchRecord->size);
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
    XenRef<xen_pool_patch, xen_pool_patch_free_t> patchRefid;
    if (!xen_pool_patch_get_by_uuid(_session, &patchRefid, const_cast<char*>(uuid.c_str())))
    {
        emit(ERROR);
        return false;
    }
    char* result = NULL;
    if (xen_pool_patch_apply(_session, &result, patchRefid, getXenRef()))
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


bool Host::cleanPatch(const Glib::ustring& uuid)
{
    XenRef<xen_pool_patch, xen_pool_patch_free_t> patchRefid;
    if (!xen_pool_patch_get_by_uuid(_session, &patchRefid, const_cast<char*>(uuid.c_str())))
    {
        emit(ERROR);
        return false;
    }
    if (xen_pool_patch_clean(_session, patchRefid))
    {
        return true;
    }
    else
    {
        emit(ERROR);
        return false;
    }
}
