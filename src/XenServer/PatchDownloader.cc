// Copyright (C) 2012-2017 Hideaki Narita


#include "File/File.h"
#include "Logger/Trace.h"
#include "Protocol/HttpClient.h"
#include "PatchDownloader.h"


using namespace hnrt;


RefPtr<PatchDownloader> PatchDownloader::create()
{
    return RefPtr<PatchDownloader>(new PatchDownloader);
}


PatchDownloader::PatchDownloader()
{
    TRACE("PatchDownloader::ctor");
}


PatchDownloader::~PatchDownloader()
{
    TRACE("PatchDownloader::dtor");
}


void PatchDownloader::run(const Glib::ustring& url, const Glib::ustring& path)
{
    TRACE("PatchDownloader::run", "url=\"%s\" path=\"%s\"", url.c_str(), path.c_str());

    _file = File::create(path.c_str(), "w");

    if (!_file->open())
    {
        Logger::instance().error("%s: %s", _file->path(), strerror(_file->error()));
        _file.reset();
        return;
    }

    _written = 0.0;

    RefPtr<HttpClient> httpClient = HttpClient::create();
    httpClient->init();
    httpClient->setUrl(url.c_str());
    httpClient->setMethod(HttpClient::GET);
    httpClient->followLocation();
    httpClient->run(*this);

    _file->close();

    _file.reset();
}


bool PatchDownloader::onSuccess(HttpClient&, int status)
{
    Logger::instance().info("status(%d): %s %'zu bytes downloaded.", status, _file->path(), _file->nbytes());
    return true;
}


bool PatchDownloader::onFailure(HttpClient&, const char* error)
{
    Logger::instance().error("%s", error);
    return false;
}


bool PatchDownloader::onCancelled(HttpClient&)
{
    Logger::instance().warn("Patch download cancelled.");
    return false;
}


bool PatchDownloader::write(HttpClient&, void* ptr, size_t len)
{
    TRACE("PatchDownloader::write", "len=%zu", len);

    if (len)
    {
        if (_file->write(ptr, len))
        {
            Glib::Mutex::Lock lock(_mutex);
            _written += static_cast<double>(len);
        }
        else
        {
            return false;
        }
    }

    return true;
}
