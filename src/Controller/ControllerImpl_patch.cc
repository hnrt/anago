// Copyright (C) 2012-2017 Hideaki Narita


#include "Base/StringBuffer.h"
#include "File/File.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Model/PatchRecord.h"
#include "Protocol/ThinClientInterface.h"
#include "XenServer/Host.h"
#include "XenServer/Session.h"
#include "XenServer/PatchDownloader.h"
#include "XenServer/PatchUploader.h"
#include "ControllerImpl.h"


using namespace hnrt;


void ControllerImpl::browsePatchPage(const Glib::ustring& uuid)
{
    TRACE("ControllerImpl::browsePatchPage", "uuid=%s", uuid.c_str());
    RefPtr<Host> host = Model::instance().getSelectedHost();
    if (!host)
    {
        return;
    }
    RefPtr<PatchRecord> record = host->getPatchRecord(uuid);
    if (!record)
    {
        return;
    }
    Glib::ustring path = Model::instance().getWebBrowserPath();
    Glib::ustring::size_type pos = path.rfind('/');
    Glib::ustring name = pos == Glib::ustring::npos ? path : path.substr(pos + 1);
    pid_t pid = fork();
    if (!pid)
    {
        execl(path.c_str(), name.c_str(), record->url.c_str(), NULL);
        _exit(1);
    }
}


void ControllerImpl::downloadPatch(const Glib::ustring& uuid)
{
    TRACE("ControllerImpl::downloadPatch", "uuid=%s", uuid.c_str());
    RefPtr<Host> host = Model::instance().getSelectedHost();
    if (!host)
    {
        return;
    }
    RefPtr<PatchRecord> record = host->getPatchRecord(uuid);
    if (!record)
    {
        return;
    }
    schedule(sigc::bind<RefPtr<Host>, RefPtr<PatchRecord> >(sigc::mem_fun(*this, &ControllerImpl::downloadPatchInBackground), host, record));
}


void ControllerImpl::downloadPatchInBackground(RefPtr<Host> host, RefPtr<PatchRecord> patchRecord)
{
    TRACE("ControllerImpl::downloadPatchInBackground");
    patchRecord->state = PatchState::DOWNLOAD_INPROGRESS;
    host->emit(XenObject::RECORD_UPDATED);
    Glib::ustring dir = Model::instance().getAppDir();
    Glib::ustring path;
    Glib::ustring::size_type pos1 = patchRecord->patchUrl.rfind('/');
    if (pos1 != Glib::ustring::npos)
    {
        Glib::ustring::size_type pos2 = patchRecord->patchUrl.find('&', pos1 + 1);
        Glib::ustring::size_type len = (pos2 != Glib::ustring::npos) ? (pos2 - (pos1 + 1)) : Glib::ustring::npos;
        path = Glib::ustring::compose("%1%2", dir, patchRecord->patchUrl.substr(pos1 + 1, len));
    }
    else
    {
        path = Glib::ustring::compose("%1%2.zip", dir, patchRecord->label);
    }
    TRACEPUT("url=\"%s\" path=\"%s\"", patchRecord->patchUrl.c_str(), path.c_str());
    RefPtr<PatchDownloader> downloader = PatchDownloader::create();
    downloader->run(patchRecord->patchUrl, path);
    RefPtr<File> file = File::create(path.c_str());
    if (!file->exists())
    {
        Logger::instance().warn("%s: Not found. Download failed.", file->path());
        patchRecord->state = PatchState::DOWNLOAD_FAILURE;
        host->emit(XenObject::RECORD_UPDATED);
        return;
    }
    else if (!file->size())
    {
        Logger::instance().warn("%s: Empty. Download failed.", file->path());
        patchRecord->state = PatchState::DOWNLOAD_FAILURE;
        host->emit(XenObject::RECORD_UPDATED);
        return;
    }
    RefPtr<File> file2 = patchRecord->unzipFile(path);
    if (!file2)
    {
        Logger::instance().warn("%s: No patch file was found. Treated as download failure.", path.c_str());
        patchRecord->state = PatchState::DOWNLOAD_FAILURE;
        host->emit(XenObject::RECORD_UPDATED);
        return;
    }
    else if (!file2->size())
    {
        Logger::instance().warn("%s: Empty. Download failed.", file2->path());
        patchRecord->state = PatchState::DOWNLOAD_FAILURE;
        host->emit(XenObject::RECORD_UPDATED);
        return;
    }
    Logger::instance().info("%s: %'zu bytes extracted.", file2->path(), file2->size());
    patchRecord->state = PatchState::DOWNLOADED;
    host->emit(XenObject::RECORD_UPDATED);
}


void ControllerImpl::uploadPatch(const Glib::ustring& uuid)
{
    TRACE("ControllerImpl::uploadPatch", "uuid=%s", uuid.c_str());
    RefPtr<Host> host = Model::instance().getSelectedHost();
    if (!host)
    {
        return;
    }
    RefPtr<PatchRecord> record = host->getPatchRecord(uuid);
    if (!record)
    {
        return;
    }
    schedule(sigc::bind<RefPtr<Host>, RefPtr<PatchRecord> >(sigc::mem_fun(*this, &ControllerImpl::uploadPatchInBackground), host, record));
}


void ControllerImpl::uploadPatchInBackground(RefPtr<Host> host, RefPtr<PatchRecord> patchRecord)
{
    TRACE("ControllerImpl::uploadPatchInBackground");
    patchRecord->state = PatchState::UPLOAD_INPROGRESS;
    host->emit(XenObject::RECORD_UPDATED);
    RefPtr<File> file = patchRecord->getFile();
    if (!file)
    {
        Logger::instance().warn("%s: No patch file was found. Treated as upload failure.", patchRecord->label.c_str());
        patchRecord->state = PatchState::UPLOAD_FAILURE;
        host->emit(XenObject::RECORD_UPDATED);
        return;
    }
    TRACEPUT("path=\"%s\"", file->path());
    RefPtr<PatchUploader> uploader = PatchUploader::create();
    if (uploader->run(host->getSession(), file->path()))
    {
        patchRecord->state = PatchState::UPLOADED;
    }
    else
    {
        patchRecord->state = PatchState::UPLOAD_FAILURE;
    }
    host->updatePatchList();
    host->emit(XenObject::RECORD_UPDATED);
}


void ControllerImpl::applyPatch(const Glib::ustring& uuid)
{
    TRACE("ControllerImpl::applyPatch", "uuid=%s", uuid.c_str());
    RefPtr<Host> host = Model::instance().getSelectedHost();
    if (!host)
    {
        return;
    }
    RefPtr<PatchRecord> record = host->getPatchRecord(uuid);
    if (!record)
    {
        return;
    }
    schedule(sigc::bind<RefPtr<Host>, RefPtr<PatchRecord> >(sigc::mem_fun(*this, &ControllerImpl::applyPatchInBackground), host, record));
}


void ControllerImpl::applyPatchInBackground(RefPtr<Host> host, RefPtr<PatchRecord> patchRecord)
{
    TRACE("ControllerImpl::applyPatchInBackground");
    patchRecord->state = PatchState::APPLY_INPROGRESS;
    host->emit(XenObject::RECORD_UPDATED);
    XenObject::Busy busy(*host);
    Session::Lock lock(host->getSession());
    const ConnectSpec& cs = host->getSession().getConnectSpec();
    Glib::ustring pw = cs.descramblePassword();
    RefPtr<ThinClientInterface> cli = ThinClientInterface::create();
    cli->init();
    cli->setHostname(cs.hostname.c_str());
    cli->setUsername(cs.username.c_str());
    cli->setPassword(pw.c_str());
    bool result = cli->run("update-pool-apply",
                           StringBuffer().format("uuid=%s", patchRecord->uuid.c_str()).str(),
                           NULL);
    pw.clear();
    patchRecord->state = result ? PatchState::APPLIED : PatchState::APPLY_FAILURE;
    host->emit(XenObject::RECORD_UPDATED);
}


void ControllerImpl::cleanPatch(const Glib::ustring& uuid)
{
    TRACE("ControllerImpl::cleanPatch", "uuid=%s", uuid.c_str());
    RefPtr<Host> host = Model::instance().getSelectedHost();
    if (!host)
    {
        return;
    }
    RefPtr<PatchRecord> record = host->getPatchRecord(uuid);
    if (!record)
    {
        return;
    }
    schedule(sigc::bind<RefPtr<Host>, RefPtr<PatchRecord> >(sigc::mem_fun(*this, &ControllerImpl::cleanPatchInBackground), host, record));
}


void ControllerImpl::cleanPatchInBackground(RefPtr<Host> host, RefPtr<PatchRecord> patchRecord)
{
    TRACE("ControllerImpl::cleanPatchInBackground");
    patchRecord->state = PatchState::CLEAN_INPROGRESS;
    host->emit(XenObject::RECORD_UPDATED);
    XenObject::Busy busy(*host);
    Session::Lock lock(host->getSession());
    bool result = host->cleanPatch(patchRecord->uuid);
    patchRecord->state = result ? PatchState::CLEANED : PatchState::CLEAN_FAILURE;
    host->updatePatchList();
    host->emit(XenObject::RECORD_UPDATED);
}
