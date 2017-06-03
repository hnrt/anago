// Copyright (C) 2012-2017 Hideaki Narita


#include <curl/curl.h>
#include "File/File.h"
#include "Logger/Logger.h"
#include "XenServer/Host.h"
#include "XenServer/Session.h"
#include "PatchUploader.h"


using namespace hnrt;


RefPtr<PatchUploader> PatchUploader::create()
{
    return RefPtr<PatchUploader>(new PatchUploader);
}


PatchUploader::PatchUploader()
{
    init();
}


PatchUploader::~PatchUploader()
{
    fini();
}


void PatchUploader::init()
{
}


void PatchUploader::fini()
{
}


static curlioerr IoControl(CURL *handle, curliocmd cmd, PatchUploader* pThis)
{
    (void)handle;

    Logger::instance().trace("IoControl(%d)", (int)cmd);

    switch (cmd)
    {
    case CURLIOCMD_RESTARTREAD:
        pThis->rewind();
        break;

    default:
        return CURLIOE_UNKNOWNCMD;
    }

    return CURLIOE_OK;
}


static size_t SendData(void* ptr, size_t size, size_t nmemb, PatchUploader* pThis)
{
    size_t len = size * nmemb;
    size_t ret = pThis->read(ptr, len);
    return ret;
}


void PatchUploader::run(Host& host, const Glib::ustring& path)
{
    _file = File::create(path.c_str(), "r");

    Session& session = host.getSession();

    CURL* curl = NULL;
    struct curl_slist *chunk = NULL;

    try
    {
        if (!session.isConnected())
        {
            throw "session";
        }

        if (!_file->open())
        {
            throw "fopen";
        }

        curl = curl_easy_init();
        if (!curl)
        {
            throw "curl_easy_init";
        }

        ConnectSpec& cs = session.getConnectSpec();
        Glib::ustring url = Glib::ustring::compose("http://%1/pool_patch_upload?session_id=%2", cs.hostname, session->session_id);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_PUT, 1L);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, _file->size());
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, SendData);
        curl_easy_setopt(curl, CURLOPT_READDATA, this);
        curl_easy_setopt(curl, CURLOPT_IOCTLFUNCTION, IoControl);
        curl_easy_setopt(curl, CURLOPT_IOCTLDATA, this);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);

        chunk = curl_slist_append(chunk, "Expect:");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        CURLcode result = curl_easy_perform(curl);

        _file->close();

        if (result != CURLE_OK)
        {
            if (result == CURLE_ABORTED_BY_CALLBACK)
            {
                goto done;
            }
            Logger::instance().error("CURL: %d (%s)", (int)result, curl_easy_strerror(result));
            throw "curl_easy_perform";
        }
    }
    catch (...)
    {
    }

done:

    if (chunk)
    {
        curl_slist_free_all(chunk);
    }

    if (curl)
    {
        curl_easy_cleanup(curl);
    }

    _file.reset();
}


size_t PatchUploader::read(void* ptr, size_t len)
{
    size_t ret = _file->read(ptr, len);
    if (!ret)
    {
        ret = CURL_READFUNC_ABORT;
    }
    return ret;
}


void PatchUploader::rewind()
{
    _file->rewind();
}


void PatchUploader::updateStatus()
{
}
