// Copyright (C) 2012-2017 Hideaki Narita


#include "File/File.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Model/PatchRecord.h"
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
    if (!file->size())
    {
        patchRecord->state = PatchState::DOWNLOAD_FAILURE;
        host->emit(XenObject::RECORD_UPDATED);
        return;
    }
    Glib::ustring filename = Glib::ustring::compose("%1.xsupdate", patchRecord->label);
    Glib::ustring cmd = Glib::ustring::compose("unzip \"%1\" \"%2\" -d \"%3\"", path, filename, dir);
    TRACEPUT("cmd=\"%s\"", cmd.c_str());
    system(cmd.c_str());
    Glib::ustring path2 = Glib::ustring::compose("%1%2", dir, filename);
    RefPtr<File> file2 = File::create(path2.c_str());
    if (!file2->size())
    {
        patchRecord->state = PatchState::DOWNLOAD_FAILURE;
        host->emit(XenObject::RECORD_UPDATED);
        return;
    }
    patchRecord->state = PatchState::DOWNLOADED;
    host->emit(XenObject::RECORD_UPDATED);
    TRACEPUT("%s: %zu bytes extracted.", path2.c_str(), file2->size());
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
    Glib::ustring filename = Glib::ustring::compose("%1.xsupdate", patchRecord->label);
    Glib::ustring dir = Model::instance().getAppDir();
    Glib::ustring path = Glib::ustring::compose("%1%2", dir, filename);
    TRACEPUT("path=\"%s\"", path.c_str());
    RefPtr<PatchUploader> uploader = PatchUploader::create();
    uploader->run(*host, path);
    sleep(1); // in order for host to finish updating its internal patch information
    patchRecord->state = PatchState::UPLOADED;
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
    bool result = host->applyPatch(patchRecord->uuid);
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
