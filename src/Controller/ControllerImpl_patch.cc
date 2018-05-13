// Copyright (C) 2012-2018 Hideaki Narita


#include <libintl.h>
#include "Base/StringBuffer.h"
#include "File/File.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Model/PatchRecord.h"
#include "Protocol/ThinClientInterface.h"
#include "XenServer/Host.h"
#include "XenServer/Session.h"
#include "XenServer/Patch.h"
#include "ControllerImpl.h"


using namespace hnrt;


void ControllerImpl::browsePatchPage(const Glib::ustring& uuid)
{
    Trace trace(NULL, "ControllerImpl::browsePatchPage(%s)", uuid.c_str());
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
    Trace trace(NULL, "ControllerImpl::downloadPatch(%s)", uuid.c_str());
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
    RefPtr<Patch> patch = Patch::create(host->getSession(), record);
    schedule(sigc::bind<RefPtr<Patch> >(sigc::mem_fun(*this, &ControllerImpl::downloadPatchInBackground), patch));
}


void ControllerImpl::downloadPatchInBackground(RefPtr<Patch> patch)
{
    Trace trace(NULL, "ControllerImpl::downloadPatchInBackground");
    patch->initDownload();
    patch->download();
    patch->fini();
}


void ControllerImpl::uploadPatch(const Glib::ustring& uuid)
{
    Trace trace(NULL, "ControllerImpl::uploadPatch(%s)", uuid.c_str());
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
    RefPtr<Patch> patch = Patch::create(host->getSession(), record);
    schedule(sigc::bind<RefPtr<Patch> >(sigc::mem_fun(*this, &ControllerImpl::uploadPatchInBackground), patch));
}


void ControllerImpl::uploadPatchInBackground(RefPtr<Patch> patch)
{
    Trace trace(NULL, "ControllerImpl::uploadPatchInBackground(%s)", patch->getRecord()->uuid.c_str());
    patch->init();
    patch->upload();
    patch->fini();
}


void ControllerImpl::applyPatch(const Glib::ustring& uuid)
{
    Trace trace(NULL, "ControllerImpl::applyPatch(%s)", uuid.c_str());
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
    RefPtr<Patch> patch = Patch::create(host->getSession(), record);
    schedule(sigc::bind<RefPtr<Patch> >(sigc::mem_fun(*this, &ControllerImpl::applyPatchInBackground), patch));
}


void ControllerImpl::applyPatchInBackground(RefPtr<Patch> patch)
{
    Trace trace(NULL, "ControllerImpl::applyPatchInBackground(%s)", patch->getRecord()->uuid.c_str());
    patch->init();
    patch->apply();
    patch->fini();
}


void ControllerImpl::cleanPatch(const Glib::ustring& uuid)
{
    Trace trace(NULL, "ControllerImpl::cleanPatch(%s)", uuid.c_str());
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
    Trace trace(NULL, "ControllerImpl::cleanPatchInBackground(%s,%s)",
                host->getSession().getConnectSpec().hostname.c_str(), patchRecord->uuid.c_str());
    patchRecord->state = PatchState::CLEAN_INPROGRESS;
    host->emit(XenObject::RECORD_UPDATED);
    XenObject::Busy busy(*host);
    Session::Lock lock(host->getSession());
    bool result = host->cleanPatch(patchRecord->uuid);
    patchRecord->state = result ? PatchState::CLEANED : PatchState::CLEAN_FAILURE;
    host->updatePatchList();
    host->emit(XenObject::RECORD_UPDATED);
}
