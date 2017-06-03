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


void PatchUploader::run(const Session& session, const Glib::ustring& path)
{
    TRACE("PatchUploader::run", "path=\"%s\"", path.c_str());

    _file = File::create(path.c_str(), "r");

    if (!_file->open())
    {
        Logger::instance().error("%s: %s", _file->path(), strerror(_file->error()));
        _file.reset();
        return;
    }

    const ConnectSpec& cs = session.getConnectSpec();
    Glib::ustring url = Glib::ustring::compose("http://%1/pool_patch_upload?session_id=%2", cs.hostname, session->session_id);

    RefPtr<HttpClient> httpClient = HttpClient::create();
    httpClient->init();
    httpClient->setUrl(url.c_str());
    httpClient->setMethod(HttpClient::PUT);
    httpClient->setUpload(_file->size());
    httpClient->removeExpectHeader();
    httpClient->run(*this);

    _file->close();

    _file.reset();
}


bool PatchUploader::onSuccess(HttpClient&, int status)
{
    TRACE("PatchUploader::onSuccess", "status=%d", status);
    return true;
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
