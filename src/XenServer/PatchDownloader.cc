// Copyright (C) 2012-2017 Hideaki Narita


#include <curl/curl.h>
#include "File/File.h"
#include "Logger/Logger.h"
#include "PatchDownloader.h"


using namespace hnrt;


RefPtr<PatchDownloader> PatchDownloader::create()
{
    return RefPtr<PatchDownloader>(new PatchDownloader);
}


PatchDownloader::PatchDownloader()
{
    init();
}


PatchDownloader::~PatchDownloader()
{
    fini();
}


void PatchDownloader::init()
{
}


void PatchDownloader::fini()
{
}


static size_t receive(void* ptr, size_t size, size_t nmemb, PatchDownloader* pThis)
{
    size_t len = size * nmemb;
    size_t ret = pThis->parse(ptr, len) ? len : 0;
    return ret;
}


void PatchDownloader::run(const Glib::ustring& url0, const Glib::ustring& path)
{
    bool download = true;

    Glib::ustring url = url0;

    for (int redirection = 0; redirection < 10 && download; redirection++)
    {
        download = false;

        _file = File::create(path.c_str(), "w");

        CURL* curl = NULL;

        try
        {
            if (!_file->open())
            {
                throw "fopen";
            }

            curl = curl_easy_init();
            if (!curl)
            {
                throw "curl_easy_init";
            }

            _context = curl;
            _contentLength = -1.0;
            _written = 0.0;

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);

            CURLcode result = curl_easy_perform(curl);

            _file->close();

            if (result != CURLE_OK)
            {
                Logger::instance().error("CURL: %d (%s)", (int)result, curl_easy_strerror(result));
                throw "curl_easy_perform";
            }

            long responseCode = 0;
            result = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
            if (result != CURLE_OK)
            {
                Logger::instance().error("CURL_getinfo(RESPONSE_CODE): %d (%s)", (int)result, curl_easy_strerror(result));
                throw "curl_easy_getinfo(RESPONSE_CODE)";
            }

            if (responseCode == 200)
            {
                Logger::instance().info("%s\t%'zu bytes downloaded.", _file->path(), _file->nbytes());
            }
            else if (responseCode / 100 == 3)
            {
                char* location = NULL;
                result = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &location);
                if (result != CURLE_OK)
                {
                    Logger::instance().error("CURL_getinfo(REDIRECT_URL): %d (%s)", (int)result, curl_easy_strerror(result));
                    throw "curl_easy_getinfo(REDIRECT_URL)";
                }
                else if (!location)
                {
                    Logger::instance().error("CURL: Response=%d Location=(null)", (int)responseCode);
                    throw "curl_easy_getinfo(REDIRECT_URL) returns NULL";
                }
                Logger::instance().info("CURL: Response=%d Location=%s", (int)responseCode, location);
                url = location;
                download = true;
            }
            else
            {
                Logger::instance().error("CURL: Response=%d\n", (int)responseCode);
            }
        }
        catch (...)
        {
        }

        if (curl)
        {
            curl_easy_cleanup(curl);
        }

        _file.reset();
    }
}


bool PatchDownloader::parse(void* ptr, size_t len)
{
    if (_contentLength < 0.0)
    {
        Glib::Mutex::Lock lock(_mutex);
        CURLcode result = curl_easy_getinfo(reinterpret_cast<CURL*>(_context), CURLINFO_CONTENT_LENGTH_DOWNLOAD, &_contentLength);
        if (result == CURLE_OK)
        {
            if (_contentLength < 0.0)
            {
                _contentLength = 0.0;
            }
        }
        else
        {
            Logger::instance().error("CURL_getinfo(CONTENT_LENGTH_DOWNLOAD): %d (%s)", (int)result, curl_easy_strerror(result));
            _contentLength = 0.0;
        }
    }

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
