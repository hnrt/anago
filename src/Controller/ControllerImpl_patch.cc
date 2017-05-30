// Copyright (C) 2012-2017 Hideaki Narita


#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Model/PatchRecord.h"
#include "XenServer/Host.h"
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
