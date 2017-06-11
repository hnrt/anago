// Copyright (C) 2017 Hideaki Narita


#include <string.h>
#include <stdexcept>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "HttpClientDefaultHandler.h"
#include "HttpClientImpl.h"


using namespace hnrt;


HttpClientImpl::HttpClientImpl()
    : _curl(NULL)
    , _headers(NULL)
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

    if (_curl)
    {
        fini();
    }
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

    if (_headers)
    {
        curl_slist_free_all(_headers);
        _headers = NULL;
    }

    if (_curl)
    {
        curl_easy_cleanup(_curl);
        _curl = NULL;
    }
}


void HttpClientImpl::setMaxConnects(int count)
{
    TRACE1("HttpClientImpl@%zx: MAXCONNECTS=%d", this, count);
    curl_easy_setopt(_curl, CURLOPT_MAXCONNECTS, static_cast<long>(count));
}


void HttpClientImpl::setFreshConnect(bool enabled)
{
    TRACE1("HttpClientImpl@%zx: FRESH_CONNECT=%d", this, enabled ? 1 : 0);
    curl_easy_setopt(_curl, CURLOPT_FRESH_CONNECT, enabled ? 1L : 0L);
}


void HttpClientImpl::setForbidReuse(bool enabled)
{
    TRACE1("HttpClientImpl@%zx: FORBID_REUSE=%d", this, enabled ? 1 : 0);
    curl_easy_setopt(_curl, CURLOPT_FORBID_REUSE, enabled ? 1L : 0L);
}


void HttpClientImpl::setHttpVersion(const char* version)
{
    if (!strcmp(version, "1.1"))
    {
        TRACE1("HttpClientImpl@%zx: HTTP_VERSION=1.1", this);
        curl_easy_setopt(_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    }
    else if (!strcmp(version, "1.0"))
    {
        TRACE1("HttpClientImpl@%zx: HTTP_VERSION=1.0", this);
        curl_easy_setopt(_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    }
    else
    {
        throw std::invalid_argument("HttpClientImpl::setHttpVersion");
    }
}


void HttpClientImpl::setUrl(const char* url)
{
    TRACE1("HttpClientImpl@%zx: URL=%s", this, url);
    curl_easy_setopt(_curl, CURLOPT_URL, url);
}


void HttpClientImpl::setMethod(Method method)
{
    switch (method)
    {
    case GET:
        TRACE1("HttpClientImpl@%zx: GET", this);
        break;
    case PUT:
        TRACE1("HttpClientImpl@%zx: PUT", this);
        curl_easy_setopt(_curl, CURLOPT_PUT, 1L);
        break;
    case POST:
        TRACE1("HttpClientImpl@%zx: POST", this);
        curl_easy_setopt(_curl, CURLOPT_POST, 1L);
        break;
    default:
        throw std::invalid_argument("HttpClientImpl::setMethod");
    }
}


void HttpClientImpl::setCredentials(const char* username, const char* password)
{
    TRACE1("HttpClientImpl@%zx: username=%s password=%s", this, username, password);
    curl_easy_setopt(_curl, CURLOPT_USERNAME, username);
    curl_easy_setopt(_curl, CURLOPT_PASSWORD, password);
}


void HttpClientImpl::setPost(const void* data, size_t size)
{
    TRACE1("HttpClientImpl@%zx: POST %'zu %s", this, size, (const char*)data);
    curl_easy_setopt(_curl, CURLOPT_POST, 1L);
    curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, size);
}


void HttpClientImpl::followLocation()
{
    TRACE1("HttpClientImpl@%zx: FOLLOWLOCATION=1", this);
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1L);
}


void HttpClientImpl::setUpload(size_t nbytes)
{
    TRACE1("HttpClientImpl@%zx: UPLOAD size=%'zu", this, nbytes);
    curl_easy_setopt(_curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(_curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(nbytes));
}


void HttpClientImpl::removeHeader(const char* header)
{
    TRACE1("HttpClientImpl@%zx::removeHeader(%s)", this, header);
    const char* colon = strchr(header, ':');
    _headers = curl_slist_append(_headers, colon && !colon[1] ? header : StringBuffer().format("%s:", header).str());
}


void HttpClientImpl::removeExpectHeader()
{
    removeHeader("Expect:");
}


void HttpClientImpl::setTcpNoDelay(bool enabled)
{
    TRACE1("HttpClientImpl@%zx: TCP_NODELAY=%d", this, enabled ? 1 : 0);
    curl_easy_setopt(_curl, CURLOPT_TCP_NODELAY, enabled ? 1L : 0L);
}


void HttpClientImpl::setVerbose(bool enabled)
{
    TRACE1("HttpClientImpl@%zx: VERBOSE=%d", this, enabled ? 1 : 0);
    curl_easy_setopt(_curl, CURLOPT_VERBOSE, enabled ? 1L : 0L);
    curl_easy_setopt(_curl, CURLOPT_STDERR, stderr);
}


bool HttpClientImpl::run(HttpClientHandler& handler)
{
    TRACE(StringBuffer().format("HttpClientImpl@%zx::run", this));

    _handler = &handler;
    _cancelled = false;
    _status = 0;
    _contentLength = -1.0;
    memset(_errbuf, 0, sizeof(_errbuf));

    if (_headers)
    {
        curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _headers);
    }

    _result = curl_easy_perform(_curl);

    long responseCode = 0;
    CURLcode result2 = curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &responseCode);
    if (result2 == CURLE_OK)
    {
        _status = static_cast<int>(responseCode);
        TRACEPUT("RESPONSE_CODE=%d", _status);
    }
    else
    {
        TRACEPUT("RESPONSE_CODE not available: %d (%s)", result2, curl_easy_strerror(result2));
    }

    if (_result == CURLE_OK)
    {
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

    if (_result == CURLE_ABORTED_BY_CALLBACK)
    {
        return _handler->onSuccess(*this, _status);
    }

    if (!_errbuf[0])
    {
        snprintf(_errbuf, sizeof(_errbuf), "%s", curl_easy_strerror(_result));
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
    TRACE(StringBuffer().format("HttpClientImpl@%zx::receiveData", pThis), "size=%zu nmemb=%zu", size, nmemb);

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
    TRACE(StringBuffer().format("HttpClientImpl@%zx::sendData", pThis), "size=%zu nmemb=%zu", size, nmemb);

    if (pThis->_cancelled)
    {
        TRACEPUT("Cancelled by user.");
        return CURL_READFUNC_ABORT;
    }

    size_t len = size * nmemb;
    if (!len)
    {
        TRACEPUT("return=0");
        return 0;
    }

    size_t ret = pThis->_handler->read(*pThis, ptr, len);
    TRACEPUT("read=%zu", ret);
    if (ret)
    {
        TRACEPUT("return=%'zu", ret / size);
        return ret / size;
    }
    else
    {
        // In the scenario to upload data to XenServer,
        // CURL will freeze after this callback returns zero.
        // To avoid such situation, forcibly end the session by returning this:
        TRACEPUT("Cancelled for workaround.");
        return CURL_READFUNC_ABORT;
    }
}


bool HttpClientImpl::connect()
{
    curl_easy_setopt(_curl, CURLOPT_CONNECT_ONLY, 1L);
    HttpClientDefaultHandler handler;
    return run(handler);
}


int HttpClientImpl::getSocket() const
{
    long lastSocket = -1;
    const_cast<HttpClientImpl*>(this)->_result = curl_easy_getinfo(_curl, CURLINFO_LASTSOCKET, &lastSocket);
    if (_result != CURLE_OK)
    {
        return -1;
    }
    else if (0 < lastSocket && lastSocket <= INT_MAX)
    {
        return static_cast<int>(lastSocket);
    }
    else
    {
        return -1;
    }
}


ssize_t HttpClientImpl::recv(void* ptr, size_t len)
{
    size_t n = 0;
    _result = curl_easy_recv(_curl, ptr, len, &n);
    if (_result == CURLE_OK)
    {
        return n;
    }
    else if (_result == CURLE_AGAIN)
    {
        return 0;
    }
    else
    {
        snprintf(_errbuf, sizeof(_errbuf), "%s", curl_easy_strerror(_result));
        return -1;
    }
}


ssize_t HttpClientImpl::send(const void* ptr, size_t len)
{
    size_t n = 0;
    _result = curl_easy_send(_curl, ptr, len, &n);
    if (_result == CURLE_OK)
    {
        return n;
    }
    else if (_result == CURLE_AGAIN)
    {
        return 0;
    }
    else
    {
        snprintf(_errbuf, sizeof(_errbuf), "%s", curl_easy_strerror(_result));
        return -1;
    }
}
