// Copyright (C) 2017 Hideaki Narita


#include <string.h>
#include <stdexcept>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "HttpClientImpl.h"


using namespace hnrt;


HttpClientImpl::HttpClientImpl()
    : _curl(NULL)
    , _chunk(NULL)
    , _handler(NULL)
    , _cancelled(false)
    , _status(0)
    , _contentLength(-1.0)
{
    TRACE(StringBuffer().format("HttpClientImpl@%zx::ctor", this));
}


HttpClientImpl::~HttpClientImpl()
{
    TRACE(StringBuffer().format("HttpClientImpl@%zx::dtor", this));

    fini();
}


void HttpClientImpl::init()
{
    TRACE(StringBuffer().format("HttpClientImpl@%zx::init", this));

    _curl = curl_easy_init();
    if (!_curl)
    {
        throw std::runtime_error("curl_easy_init");
    }

    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, receiveData);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(_curl, CURLOPT_READFUNCTION, sendData);
    curl_easy_setopt(_curl, CURLOPT_READDATA, this);
    curl_easy_setopt(_curl, CURLOPT_IOCTLFUNCTION, ioControl);
    curl_easy_setopt(_curl, CURLOPT_IOCTLDATA, this);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(_curl, CURLOPT_ERRORBUFFER, _errbuf);
}


void HttpClientImpl::fini()
{
    TRACE(StringBuffer().format("HttpClientImpl@%zx::fini", this));

    if (_chunk)
    {
        curl_slist_free_all(_chunk);
        _chunk = NULL;
    }

    if (_curl)
    {
        curl_easy_cleanup(_curl);
        _curl = NULL;
    }
}


void HttpClientImpl::setUrl(const char* url)
{
    curl_easy_setopt(_curl, CURLOPT_URL, url);
}


void HttpClientImpl::setMethod(Method method)
{
    switch (method)
    {
    case GET:
        break;
    case PUT:
        curl_easy_setopt(_curl, CURLOPT_PUT, 1L);
        break;
    case POST:
        curl_easy_setopt(_curl, CURLOPT_POST, 1L);
        break;
    default:
        throw std::runtime_error("HttpClientImpl::setMethod");
    }
}


void HttpClientImpl::followLocation()
{
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1L);
}


void HttpClientImpl::setUpload(size_t nbytes)
{
    curl_easy_setopt(_curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(_curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(nbytes));
}


void HttpClientImpl::removeExpectHeader()
{
    _chunk = curl_slist_append(_chunk, "Expect:");
}


bool HttpClientImpl::run(HttpClientHandler& handler)
{
    TRACE(StringBuffer().format("HttpClientImpl@%zx::run", this));

    _handler = &handler;
    _cancelled = false;
    _status = 0;
    _contentLength = -1.0;
    memset(_errbuf, 0, sizeof(_errbuf));

    if (_chunk)
    {
        curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _chunk);
    }

    CURLcode result = curl_easy_perform(_curl);

    if (result == CURLE_OK)
    {
        long value = 0;
        CURLcode result = curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &value);
        if (result == CURLE_OK)
        {
            _status = static_cast<int>(value);
        }
        if (_cancelled)
        {
            return _handler->onCancelled(*this);
        }
        return _handler->onSuccess(*this, _status);
    }

    if (_cancelled)
    {
        return _handler->onCancelled(*this);
    }

    if (result == CURLE_ABORTED_BY_CALLBACK)
    {
        return _handler->onSuccess(*this, 0);
    }

    if (!_errbuf[0])
    {
        snprintf(_errbuf, sizeof(_errbuf), "%s", curl_easy_strerror(result));
    }

    return _handler->onFailure(*this, _errbuf);
}


void HttpClientImpl::cancel()
{
    _cancelled = true;
}


curlioerr HttpClientImpl::ioControl(CURL* handle, curliocmd cmd, HttpClientImpl* pThis)
{
    TRACE(StringBuffer().format("HttpClientImpl::ioControl@%zx", pThis), "cmd=%d", (int)cmd);

    (void)handle;

    switch (cmd)
    {
    case CURLIOCMD_RESTARTREAD:
        pThis->_handler->rewind(*pThis);
        break;

    default:
        return CURLIOE_UNKNOWNCMD;
    }

    return CURLIOE_OK;
}


size_t HttpClientImpl::receiveData(void* ptr, size_t size, size_t nmemb, HttpClientImpl* pThis)
{
    TRACE("HttpClientImpl::receiveData", "size=%zu nmemb=%zu", size, nmemb);

    if (pThis->_contentLength < 0.0)
    {
        CURLcode result = curl_easy_getinfo(pThis->_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &pThis->_contentLength);
        if (result != CURLE_OK)
        {
            Logger::instance().warn("CURL_getinfo(CONTENT_LENGTH_DOWNLOAD): %d (%s)", (int)result, curl_easy_strerror(result));
            pThis->_contentLength = 0.0;
        }
    }

    if (pThis->_cancelled)
    {
        return 0;
    }

    size_t len = size * nmemb;
    if (!len)
    {
        return 0;
    }

    if (pThis->_handler->write(*pThis, ptr, len))
    {
        return len / size;
    }
    else
    {
        return 0;
    }
}


size_t HttpClientImpl::sendData(void* ptr, size_t size, size_t nmemb, HttpClientImpl* pThis)
{
    TRACE("HttpClientImpl::sendData", "size=%zu nmemb=%zu", size, nmemb);

    if (pThis->_cancelled)
    {
        return CURL_READFUNC_ABORT;
    }

    size_t len = size * nmemb;
    if (!len)
    {
        return 0;
    }

    size_t ret = pThis->_handler->read(*pThis, ptr, len);
    TRACEPUT("read=%zu", ret);
    if (ret)
    {
        return ret / size;
    }
    else
    {
        // In the scenario to upload data to XenServer,
        // CURL will freeze after this callback returns zero.
        // To avoid such situation, forcibly end the session by returning this:
        return CURL_READFUNC_ABORT;
    }
}
