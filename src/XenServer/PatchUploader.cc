// Copyright (C) 2012-2017 Hideaki Narita


#include "Base/StringBuffer.h"
#include "File/File.h"
#include "Logger/Trace.h"
#include "Protocol/HttpClient.h"
#include "XenServer/Session.h"
#include "PatchUploader.h"


using namespace hnrt;


RefPtr<PatchUploader> PatchUploader::create()
{
    return RefPtr<PatchUploader>(new PatchUploader);
}


PatchUploader::PatchUploader()
{
    TRACE("PatchUploader::ctor");
}


PatchUploader::~PatchUploader()
{
    TRACE("PatchUploader::dtor");
}


bool PatchUploader::run(Session& session, const char* path)
{
    TRACE("PatchUploader::run", "path=\"%s\"", path);

    bool retval = false;

    {
        _file = File::create(path, "r");

        if (!_file->open())
        {
            Logger::instance().error("%s: %s", _file->path(), strerror(_file->error()));
            goto done;
        }

        XenRef<xen_task, xen_task_free_t> task;
        char name[] = { "pool_patch_upload" };
        char desc[] = { "" };
        if (!xen_task_create(session, &task, name, desc))
        {
            Logger::instance().error("xen_task_create failed.");
            session.clearError();
            goto done;
        }
        char* uuidTask = NULL;
        xen_task_get_uuid(session, &uuidTask, task);
        char* uuidSession = NULL;
        xen_session_get_uuid(session, &uuidSession, session);

        const ConnectSpec& cs = session.getConnectSpec();
        Glib::ustring url = Glib::ustring::compose(
            "https://%1/pool_patch_upload?task_id=%2&session_id=%3",
            cs.hostname,
            uuidTask,
            uuidSession);

        xen_uuid_free(uuidTask);
        xen_uuid_free(uuidSession);

        Glib::ustring pw = cs.descramblePassword();
        printf("password=\"%s\"\n", pw.c_str());

        RefPtr<HttpClient> httpClient = HttpClient::create();
        httpClient->init();
        httpClient->setUrl(url.c_str());
        httpClient->setMethod(HttpClient::PUT);
        httpClient->setCredentials(cs.username.c_str(), pw.c_str());
        httpClient->setUpload(_file->size());
        httpClient->removeExpectHeader();
        httpClient->setVerbose(Logger::instance().getLevel() <= LogLevel::TRACE ? true : false);
        retval = httpClient->run(*this);

        pw.clear();

        if (!xen_task_destroy(session, task))
        {
            Logger::instance().error("xen_task_destroy failed.");
            session.clearError();
            goto done;
        }

        _file->close();
    }

done:

    _file.reset();

    return retval;
}


bool PatchUploader::onSuccess(HttpClient&, int status)
{
    TRACE("PatchUploader::onSuccess", "status=%d", status);
    return status == 200 ? true : false;
}


bool PatchUploader::onFailure(HttpClient&, const char* error)
{
    Logger::instance().error("%s", error);
    return false;
}


bool PatchUploader::onCancelled(HttpClient&)
{
    Logger::instance().warn("Patch upload cancelled.");
    return false;
}


size_t PatchUploader::read(HttpClient&, void* ptr, size_t len)
{
    return _file->read(ptr, len);
}


void PatchUploader::rewind(HttpClient&)
{
    _file->rewind();
}
