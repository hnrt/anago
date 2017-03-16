// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <time.h>
//#include "PingAgent.h"
//#include "PatchBase.h"
//#include "PatchRecord.h"
#include "Base/Atomic.h"
#include "Controller/Controller.h"
#include "Logger/Trace.h"
#include "Host.h"
#include "Session.h"
#include "XenRef.h"


using namespace hnrt;


#ifdef _DEBUG_HOST
#define DBG(format,...) Debug::putMF(__PRETTY_FUNCTION__,this,format,##__VA_ARGS__)
#define DBG2(format,...) Debug::put(__FUNCTION__,format,##__VA_ARGS__)
#else
#define DBG(format,...) (void)0
#define DBG2(format,...) (void)0
#endif


RefPtr<Host> Host::create(Session& session)
{
    RefPtr<Host> host(new Host(session));
    return host;
}


Host::Host(Session& session)
    : XenObject(XenObject::HOST, session, NULL, NULL, NULL)
    , _state(STATE_NONE)
  //, _ping(PingAgent::create())
{
    Trace trace(__PRETTY_FUNCTION__);

    //_ping->open(_session.getConnectSpec().hostname);
}


Host:: ~Host()
{
    Trace trace(__PRETTY_FUNCTION__);

    //_ping->close();
}


void Host::setBusy(bool value)
{
    int count;
    if (value)
    {
        count = InterlockedIncrement(&_busyCount);
        if (count != 1)
        {
            return;
        }
    }
    else
    {
        count = InterlockedDecrement(&_busyCount);
        if (count != 0)
        {
            return;
        }
    }
    Controller::instance().notify(RefPtr<RefObj>(this, 1), Controller::XO_BUSY);
    if (count == 0)
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
}


XenPtr<xen_host_record> Host::getRecord()
{
    Glib::RecMutex::Lock k(_mutex);
    return _record;
}


void Host::setRecord(const XenPtr<xen_host_record>& record)
{
    if (record)
    {
        Glib::RecMutex::Lock k(_mutex);
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
    Controller::instance().notify(RefPtr<RefObj>(this, 1), Controller::XO_RECORD);
}


XenPtr<xen_host_metrics_record> Host::getMetricsRecord()
{
    Glib::RecMutex::Lock k(_mutex);
    return _metricsRecord;
}


void Host::setMetricsRecord(const XenPtr<xen_host_metrics_record>& record)
{
    if (record)
    {
        Glib::RecMutex::Lock k(_mutex);
        _metricsRecord = record;
    }
    else
    {
        return;
    }
    Controller::instance().notify(RefPtr<RefObj>(this, 1), Controller::XO_RECORD);
}


void Host::onConnectPending()
{
    _state = STATE_CONNECT_PENDING;
    setDisplayStatus(gettext("Connecting..."));
}


void Host::onConnected()
{
    _state = STATE_CONNECTED;
    _session.getConnectSpec().lastAccess = (long)time(NULL);
    setDisplayStatus(gettext("Connected"));
    //_ping->close();
    XenRef<xen_host, xen_host_free_t> host;
    if (!xen_session_get_this_host(_session, &host, _session))
    {
        return;
    }
    _refid = host.toString();
    XenPtr<xen_host_record> record;
    if (!xen_host_get_record(_session, record.address(), host))
    {
        return;
    }
    XenPtr<xen_host_metrics_record> metricsRecord;
    XenRef<xen_host_metrics, xen_host_metrics_free_t> metrics;
    if (xen_host_get_metrics(_session, &metrics, host))
    {
        if (xen_host_metrics_get_record(_session, metricsRecord.address(), metrics))
        {
        }
        else
        {
            g_print("Host::connect: xen_host_metrics_get_record failed.\n");
            _session.clearError();
        }
    }
    else
    {
        g_print("Host::connect: xen_host_get_metrics failed.\n");
        _session.clearError();
    }
    //getMac();
    {
        Glib::RecMutex::Lock k(_mutex);
        _record = record;
        _metricsRecord = metricsRecord;
        _uuid = record->uuid ? record->uuid : "";
        _name = record->name_label ? record->name_label : "";
    }
    //initPatchList();
    //updatePatchList();
    Controller::instance().notify(RefPtr<RefObj>(this, 1), Controller::XO_SESSION);
}


void Host::onConnectFailed()
{
    _state = STATE_CONNECT_FAILED;
    setDisplayStatus(gettext("Failed to connect"));
}


void Host::onDisconnected()
{
    Controller::instance().notify(RefPtr<RefObj>(this, 1), Controller::XO_SESSION);
    if (_state == STATE_CONNECTED || _state == STATE_DISCONNECT_PENDING)
    {
        setDisplayStatus(gettext("Disconnected"));
    }
    else if (_state == STATE_DISCONNECTED_BY_PEER)
    {
        setDisplayStatus(gettext("Disconnected by peer"));
    }
    //_patchList.clear();
    _state = STATE_DISCONNECTED;
    //if (!_ping->active())
    //{
    //    _ping->open();
    //}
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
        if (xen_host_disable(_session, (xen_host)_refid.c_str()) &&
            xen_host_shutdown(_session, (xen_host)_refid.c_str()))
        {
            setDisplayStatus(gettext("Shut down successfully"));
            _state = STATE_SHUTDOWN;
            return true;
        }
        else
        {
            setDisplayStatus(gettext("Failed to shut down"));
            _state = STATE_SHUTDOWN_FAILED;
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
        if (xen_host_disable(_session, (xen_host)_refid.c_str()) &&
            xen_host_reboot(_session, (xen_host)_refid.c_str()))
        {
            setDisplayStatus(gettext("Rebooted successfully"));
            _state = STATE_REBOOTED;
            return true;
        }
        else
        {
            setDisplayStatus(gettext("Failed to reboot"));
            _state = STATE_REBOOT_FAILED;
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
    if (!_session)
    {
        return false;
    }

    if (!xen_host_set_name_label(_session, (xen_host)_refid.c_str(), (char*)label))
    {
        return false;
    }

    if (!xen_host_set_name_description(_session, (xen_host)_refid.c_str(), (char*)description))
    {
        return false;
    }

    return true;
}


bool Host::getMac()
{
    ConnectSpec& cs = _session.getConnectSpec();
    return cs.mac.getByName(cs.hostname.c_str());
}

#if 0
bool Host::getPing()
{
    return _ping->result();
}
#endif
#if 0
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
            else if (strcasecmp((*i)->label, patchRecord->label) > 0)
            {
                RefPtr<PatchRecord> patchRecord2 = PatchRecord::create();
                *patchRecord2 = *patchRecord;
                _patchList.insert(i, patchRecord2);
                break;
            }
        }
    }
}
#endif
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
#endif
#if 0
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
#endif
#if 0
static void UpdateState(std::list<RefPtr<PatchRecord> >& patchList, XenPtr<xen_pool_patch_record> patchRecord, XenPtr<xen_host_patch_set> hostPatchSet)
{
    PatchState state;
    if (patchRecord->pool_applied)
    {
        state = PatchState::kApplied;
    }
    else if (IsApplied(patchRecord, hostPatchSet))
    {
        state = PatchState::kApplied;
    }
    else
    {
        state = PatchState::kUploaded;
    }
    for (std::list<RefPtr<PatchRecord> >::iterator i = patchList.begin(); i != patchList.end(); i++)
    {
        if (!strcmp((*i)->uuid, patchRecord->uuid))
        {
            switch ((*i)->state)
            {
            case PatchState::kAvailable:
            case PatchState::kDownloadFailure:
            case PatchState::kDownloaded:
            case PatchState::kUploadFailure:
            case PatchState::kUploaded:
            case PatchState::kApplied:
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


void Host::updatePatchList()
{
    for (std::list<RefPtr<PatchRecord> >::iterator i = _patchList.begin(); i != _patchList.end(); i++)
    {
        switch ((*i)->state)
        {
        case PatchState::kAvailable:
        case PatchState::kDownloaded:
        {
            String filename;
            filename.format("%s.xsupdate", (*i)->label.str());
            String path;
            path.format("%s/%s", Model::instance().getAppDir().str(), filename.str());
            RefPtr<File> file = File::create(path);
            (*i)->state = file->size() ? PatchState::kDownloaded : PatchState::kAvailable;
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
                    if (!strcmp((*j)->uuid, poolPatchRecord->uuid))
                    {
                        if (poolPatchRecord->size)
                        {
                            (*j)->state = PatchState::kUploaded;
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
                    if (!strcmp((*j)->uuid, poolPatchRecord->uuid))
                    {
                        (*j)->state = PatchState::kApplied;
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


RefPtr<PatchRecord> Host::getPatchRecord(String uuid) const
{
    RefPtr<PatchRecord> patchRecord;
    for (std::list<RefPtr<PatchRecord> >::const_iterator i = _patchList.begin(); i != _patchList.end(); i++)
    {
        if (!strcmp((*i)->uuid, uuid))
        {
            patchRecord = *i;
            break;
        }
    }
    return patchRecord;
}


bool Host::applyPatch(String uuid)
{
    XenRef<xen_pool_patch, xen_pool_patch_free_t> patchRefid;
    if (!xen_pool_patch_get_by_uuid(_session, &patchRefid, uuid))
    {
        return false;
    }
    char* result = NULL;
    bool retval = xen_pool_patch_apply(_session, &result, patchRefid, _refid);
    if (retval)
    {
        free(result);
    }
    return retval;
}


bool Host::cleanPatch(String uuid)
{
    XenRef<xen_pool_patch, xen_pool_patch_free_t> patchRefid;
    if (!xen_pool_patch_get_by_uuid(_session, &patchRefid, uuid))
    {
        return false;
    }
    bool retval = xen_pool_patch_clean(_session, patchRefid);
    return retval;
}
#endif
