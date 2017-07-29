// Copyright (C) 2017 Hideaki Narita


#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdexcept>
#include <sys/select.h>
#include "Logger/Trace.h"
#include "HttpClientImpl.h"


using namespace hnrt;


HttpClientImpl::HttpClientImpl()
    : _curl(NULL)
    , _headers(NULL)
    , _timeout(0)
    , _cancelled(false)
    , _socket(-1)
    , _status(0)
    , _contentLength(-1.0)
{
    TRACEFUN(this, "HttpClientImpl::ctor");
}


HttpClientImpl::~HttpClientImpl()
{
    TRACEFUN(this, "HttpClientImpl::dtor");

    if (_curl)
    {
        fini();
    }
}


void HttpClientImpl::init()
{
    TRACEFUN(this, "HttpClientImpl::init");

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
    curl_easy_setopt(_curl, CURLOPT_FRESH_CONNECT, 1);
}


void HttpClientImpl::fini()
{
    TRACEFUN(this, "HttpClientImpl::fini");

    _responseHeaderMap.clear();
    _responseBody.setLength(0);

    if (_headers)
    {
        curl_slist_free_all(_headers);
        _headers = NULL;
    }

    if (_curl)
    {
        curl_easy_cleanup(_curl);
        _curl = NULL;
        _socket = -1;
    }
}


void HttpClientImpl::setTimeout(long timeout)
{
    TRACE("HttpClientImpl@%zx: TIMEOUT=%ld", this, timeout);
    _timeout = (timeout > 0) ? timeout : 0;
}


void HttpClientImpl::setMaxConnects(int count)
{
    TRACE("HttpClientImpl@%zx: MAXCONNECTS=%d", this, count);
    curl_easy_setopt(_curl, CURLOPT_MAXCONNECTS, static_cast<long>(count));
}


void HttpClientImpl::setHttpVersion(const char* version)
{
    if (!strcmp(version, "1.1"))
    {
        TRACE("HttpClientImpl@%zx: HTTP_VERSION=1.1", this);
        curl_easy_setopt(_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    }
    else if (!strcmp(version, "1.0"))
    {
        TRACE("HttpClientImpl@%zx: HTTP_VERSION=1.0", this);
        curl_easy_setopt(_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    }
    else
    {
        throw std::invalid_argument("HttpClientImpl::setHttpVersion");
    }
}


void HttpClientImpl::setUrl(const char* url)
{
    TRACE("HttpClientImpl@%zx: URL=%s", this, url);
    curl_easy_setopt(_curl, CURLOPT_URL, url);
}


void HttpClientImpl::setMethod(Method method)
{
    switch (method)
    {
    case GET:
        TRACE("HttpClientImpl@%zx: GET", this);
        break;
    case PUT:
        TRACE("HttpClientImpl@%zx: PUT", this);
        curl_easy_setopt(_curl, CURLOPT_PUT, 1L);
        break;
    case POST:
        TRACE("HttpClientImpl@%zx: POST", this);
        curl_easy_setopt(_curl, CURLOPT_POST, 1L);
        break;
    default:
        throw std::invalid_argument("HttpClientImpl::setMethod");
    }
}


void HttpClientImpl::setCredentials(const char* username, const char* password)
{
    TRACE("HttpClientImpl@%zx: username=%s password=%s", this, username, password);
    curl_easy_setopt(_curl, CURLOPT_USERNAME, username);
    curl_easy_setopt(_curl, CURLOPT_PASSWORD, password);
}


void HttpClientImpl::setPost(const void* data, size_t size)
{
    TRACE("HttpClientImpl@%zx: POST %'zu %s", this, size, (const char*)data);
    curl_easy_setopt(_curl, CURLOPT_POST, 1L);
    curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, size);
}


void HttpClientImpl::followLocation()
{
    TRACE("HttpClientImpl@%zx: FOLLOWLOCATION=1", this);
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1L);
}


void HttpClientImpl::setUpload(size_t nbytes)
{
    TRACE("HttpClientImpl@%zx: UPLOAD size=%'zu", this, nbytes);
    curl_easy_setopt(_curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(_curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(nbytes));
}


void HttpClientImpl::removeHeader(const char* header)
{
    TRACE("HttpClientImpl@%zx::removeHeader(%s)", this, header);
    const char* colon = strchr(header, ':');
    _headers = curl_slist_append(_headers, colon && !colon[1] ? header : StringBuffer().format("%s:", header).str());
}


void HttpClientImpl::removeExpectHeader()
{
    removeHeader("Expect:");
}


void HttpClientImpl::setTcpNoDelay(bool enabled)
{
    TRACE("HttpClientImpl@%zx: TCP_NODELAY=%d", this, enabled ? 1 : 0);
    curl_easy_setopt(_curl, CURLOPT_TCP_NODELAY, enabled ? 1L : 0L);
}


void HttpClientImpl::setVerbose(bool enabled)
{
    TRACE("HttpClientImpl@%zx: VERBOSE=%d", this, enabled ? 1 : 0);
    curl_easy_setopt(_curl, CURLOPT_VERBOSE, enabled ? 1L : 0L);
    curl_easy_setopt(_curl, CURLOPT_STDERR, stderr);
}


void HttpClientImpl::setReadFunction(const ReadFunction& function)
{
    TRACE("HttpClientImpl@%zx: READFUNCTION set", this);
    _readFunction = function;
}


void HttpClientImpl::setRewindFunction(const RewindFunction& function)
{
    TRACE("HttpClientImpl@%zx: REWINDFUNCTION set", this);
    _rewindFunction = function;
}


void HttpClientImpl::setWriteFunction(const WriteFunction& function)
{
    TRACE("HttpClientImpl@%zx: WRITEFUNCTION set", this);
    _writeFunction = function;
}


void HttpClientImpl::resetReadFunction()
{
    TRACE("HttpClientImpl@%zx: READFUNCTION reset", this);
    _readFunction.disconnect();
}


void HttpClientImpl::resetRewindFunction()
{
    TRACE("HttpClientImpl@%zx: REWINDFUNCTION reset", this);
    _rewindFunction.disconnect();
}


void HttpClientImpl::resetWriteFunction()
{
    TRACE("HttpClientImpl@%zx: WRITEFUNCTION reset", this);
    _writeFunction.disconnect();
}


bool HttpClientImpl::run()
{
    TRACEFUN(this, "HttpClientImpl::run");

    _expiry.now().addMilliseconds(_timeout);
    _cancelled = false;
    _socket = -1;
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
        return true;
    }
    else
    {
        if (!_errbuf[0])
        {
            snprintf(_errbuf, sizeof(_errbuf), "%s", curl_easy_strerror(_result));
        }
        return false;
    }
}


void HttpClientImpl::cancel()
{
    _cancelled = true;
}


long HttpClientImpl::remainingTime() const
{
    if (_timeout > 0)
    {
        Time now;
        return _expiry.minus(now) / 1000L;
    }
    else
    {
        return 365L * 24L * 60L * 60L * 1000L;
    }
}


bool HttpClientImpl::timedOut() const
{
    if (_timeout > 0)
    {
        Time now;
        return _expiry <= now;
    }
    else
    {
        return false;
    }
}


curlioerr HttpClientImpl::ioControl(CURL* handle, curliocmd cmd, HttpClientImpl* pThis)
{
    TRACEFUN(pThis, "HttpClientImpl::ioControl(%d)", (int)cmd);

    (void)handle;

    switch (cmd)
    {
    case CURLIOCMD_RESTARTREAD:
        if (!pThis->_rewindFunction.empty())
        {
            pThis->_rewindFunction();
        }
        break;

    default:
        return CURLIOE_UNKNOWNCMD;
    }

    return CURLIOE_OK;
}


size_t HttpClientImpl::receiveData(void* ptr, size_t size, size_t nmemb, HttpClientImpl* pThis)
{
    if (pThis->timedOut())
    {
        TRACE("HttpClientImpl@%zx::receiveData(%zu,%zu): return=0 (timed out)", pThis, size, nmemb);
        return 0;
    }

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
        TRACE("HttpClientImpl@%zx::receiveData(%zu,%zu): return=0 (cancelled)", pThis, size, nmemb);
        return 0;
    }

    size_t len = size * nmemb;
    if (!len)
    {
        TRACE("HttpClientImpl@%zx::receiveData(%zu,%zu): return=0 (no data)", pThis, size, nmemb);
        return 0;
    }

    if (pThis->_writeFunction.empty())
    {
        if (Logger::instance().getLevel() <= LogLevel::TRACE)
        {
            StringBuffer buf;
            const char* p1 = reinterpret_cast<const char*>(ptr);
            const char* p2 = p1 + len;
            while (p1 < p2)
            {
                switch (*p1)
                {
                case '\n':
                    buf.append("\\n");
                    break;
                case '\r':
                    buf.append("\\r");
                    break;
                case '\\':
                    buf.append("\\\\");
                    break;
                default:
                    buf.append(*p1);
                    break;
                }
                p1++;
            }
            TRACE("HttpClientImpl@%zx::receiveData(%zu,%zu): %s", pThis, size, nmemb, buf.str());
        }
        return len / size;
    }
    else if (pThis->_writeFunction(ptr, len))
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
    TRACEFUN(pThis, "HttpClientImpl::sendData(size=%zu,nmemb=%zu)", size, nmemb);

    if (pThis->timedOut())
    {
        TRACE("HttpClientImpl@%zx::sendData(%zu,%zu): return=CURL_READFUNC_ABORT (timed out)", pThis, size, nmemb);
        return CURL_READFUNC_ABORT;
    }

    if (pThis->_cancelled)
    {
        TRACE("HttpClientImpl@%zx::sendData(%zu,%zu): return=CURL_READFUNC_ABORT (cancelled by user)", pThis, size, nmemb);
        return CURL_READFUNC_ABORT;
    }

    size_t len = size * nmemb;
    if (!len)
    {
        TRACE("HttpClientImpl@%zx::sendData(%zu,%zu): return=0 (no space)", pThis, size, nmemb);
        return 0;
    }

    if (pThis->_readFunction.empty())
    {
        TRACE("HttpClientImpl@%zx::sendData(%zu,%zu): return=0 (no function)", pThis, size, nmemb);
        return 0;
    }
    else
    {
        size_t ret = pThis->_readFunction(ptr, len);
        return ret / size;
    }
}


bool HttpClientImpl::connect()
{
    TRACEFUN(this, "HttpClientImpl::connect");
    curl_easy_setopt(_curl, CURLOPT_CONNECT_ONLY, 1L);
    return run();
}


int HttpClientImpl::getSocket()
{
    if (_socket < 0)
    {
        long lastSocket = -1;
        const_cast<HttpClientImpl*>(this)->_result = curl_easy_getinfo(_curl, CURLINFO_LASTSOCKET, &lastSocket);
        if (_result != CURLE_OK)
        {
            snprintf(_errbuf, sizeof(_errbuf), "%s", curl_easy_strerror(_result));
            return -1;
        }
        else if (0 < lastSocket && lastSocket <= INT_MAX)
        {
            _socket = static_cast<int>(lastSocket);
        }
        else
        {
            snprintf(_errbuf, sizeof(_errbuf), "Bad socket(%ld)", lastSocket);
            return -1;
        }
    }
    return _socket;
}


int HttpClientImpl::canRecv(long timeoutInMilliseconds)
{
    TRACEFUN(this, "HttpClientImpl::canRecv(%ld)", timeoutInMilliseconds);
    if (_socket < 0 && getSocket() < 0)
    {
        TRACEPUT("return=-1 (Socket unavailable.)");
        return -1;
    }
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(_socket, &fds);
    struct timeval timeout;
    timeout.tv_sec = timeoutInMilliseconds / 1000L;
    timeout.tv_usec = (timeoutInMilliseconds % 1000L) * 1000L;
    int rc = select(_socket + 1, &fds, NULL, NULL, &timeout);
    if (rc < 0)
    {
        snprintf(_errbuf, sizeof(_errbuf), "%s", strerror(errno));
        TRACEPUT("return=%d (%s)", rc, _errbuf);
    }
    else
    {
        TRACEPUT("return=%d", rc);
    }
    return rc;
}


int HttpClientImpl::canSend(long timeoutInMilliseconds)
{
    TRACEFUN(this, "HttpClientImpl::canSend(%ld)", timeoutInMilliseconds);
    if (_socket < 0 && getSocket() < 0)
    {
        TRACEPUT("return=-1 (Socket unavailable.)");
        return -1;
    }
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(_socket, &fds);
    struct timeval timeout;
    timeout.tv_sec = timeoutInMilliseconds / 1000L;
    timeout.tv_usec = (timeoutInMilliseconds % 1000L) * 1000L;
    int rc = select(_socket + 1, NULL, &fds, NULL, &timeout);
    if (rc < 0)
    {
        snprintf(_errbuf, sizeof(_errbuf), "%s", strerror(errno));
        TRACEPUT("return=%d (%s)", rc, _errbuf);
    }
    else
    {
        TRACEPUT("return=%d", rc);
    }
    return rc;
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


#define SP ' '
#define HT '\t'
#define CR '\r'
#define LF '\n'
#define DEL 127


inline static bool isControl(int c)
{
    return (0 <= c && c <= 31) || c == DEL;
}


inline static bool isSeparator(int c)
{
    switch (c)
    {
    case '(':
    case ')':
    case '<':
    case '>':
    case '@':
    case ',':
    case ';':
    case ':':
    case '\\':
    case '\"':
    case '/':
    case '[':
    case ']':
    case '?':
    case '=':
    case '{':
    case '}':
    case SP:
    case HT:
        return true;
    default:
        return false;
    }
}


inline static bool isWhitespace(int c)
{
    return c == SP || c == HT;
}


inline static bool isTokenChar(int c)
{
    return !isControl(c) && !isSeparator(c);
}


static bool ParseResponse(int c, int& state, HttpClientImpl::Response& response)
{
    TRACE(c >= SP ? "ParseRespose(c=%c state=%d)" : "ParseRespose(c=0x%02x state=%d)", c, state);
    switch (state)
    {
    case 0:
        response.version.major = 0;
        response.version.minor = 0;
        response.statusCode = 0;
        response.reason.setLength(0);
        response.headerMap.clear();
        response.body.setLength(0);
        if (c == 'H')
            state = 1;
        else
            return false;
        break;
    case 1:
        if (c == 'T')
            state = 2;
        else
            return false;
        break;
    case 2:
        if (c == 'T')
            state = 3;
        else
            return false;
        break;
    case 3:
        if (c == 'P')
            state = 4;
        else
            return false;
        break;
    case 4:
        if (c == '/')
            state = 5;
        else
            return false;
        break;
    case 5:
        response.val.setLength(0);
        if (isdigit(c))
        {
            response.val.append((char)c);
            state = 6;
        }
        else
            return false;
        break;
    case 6:
        if (isdigit(c))
            response.val.append((char)c);
        else if (c == '.')
        {
            response.version.major = (int)strtoul(response.val.str(), NULL, 10);
            state = 7;
        }
        else
            return false;
        break;
    case 7:
        response.val.setLength(0);
        if (isdigit(c))
        {
            response.val.append((char)c);
            state = 8;
        }
        else
            return false;
        break;
    case 8:
        if (isdigit(c))
            response.val.append((char)c);
        else if (c == SP)
        {
            response.version.minor = (int)strtoul(response.val.str(), NULL, 10);
            state = 10;
        }
        else
            return false;
        break;
    case 10:
        response.val.setLength(0);
        if (isdigit(c))
        {
            response.val.append((char)c);
            state = 11;
        }
        else
            return false;
        break;
    case 11:
        if (isdigit(c))
        {
            response.val.append((char)c);
            state = 12;
        }
        else
            return false;
        break;
    case 12:
        if (isdigit(c))
        {
            response.val.append((char)c);
            state = 13;
        }
        else
            return false;
        break;
    case 13:
        if (c == SP)
        {
            response.statusCode = (int)strtoul(response.val.str(), NULL, 10);
            state = 20;
        }
        else
            return false;
        break;
    case 20:
        if (c == CR)
            state = 21;
        else if (isWhitespace(c) || !isControl(c))
            response.reason.append((char)c);
        else
            return false;
        break;
    case 21:
        if (c == LF)
            state = 30;
        else
            return false;
        break;
    case 30:
        if (c == CR)
        {
            state = 39;
            break;
        }
        response.key.setLength(0);
        if (isTokenChar(c))
        {
            response.key.append((char)c);
            state = 31;
        }
        else
            return false;
        break;
    case 31:
        if (c == ':')
            state = 32;
        else if (isTokenChar(c))
            response.key.append((char)c);
        else
            return false;
        break;
    case 32:
        response.val.setLength(0);
        if (c == CR)
            state = 34;
        else if (isWhitespace(c))
            /* skip leading whitespaces */ ;
        else if (!isControl(c))
        {
            response.val.append((char)c);
            state = 33;
        }
        else
            return false;
        break;
    case 33:
        if (c == CR)
            state = 34;
        else if (isWhitespace(c) || !isControl(c))
            response.val.append((char)c);
        else
            return false;
        break;
    case 34:
        if (c == LF)
        {
            HttpClient::Header header;
            header.key = response.key.str();
            header.value = response.val.str();
            Glib::ustring key = header.key.lowercase();
            HttpClient::HeaderMap::iterator iter = response.headerMap.find(key);
            if (iter != response.headerMap.end())
            {
                response.headerMap.erase(iter);
            }
            response.headerMap.insert(HttpClient::HeaderMapEntry(key, header));
            state = 30;
        }
        else
            return false;
        break;
    case 39:
        if (c == LF)
        {
            state = 40;
        }
        else
            return false;
        break;
    case 40:
        response.body.append((char)c);
        break;
    default:
        return false;
    }
    return true;
}


bool HttpClientImpl::recvResponse()
{
    TRACEFUN(this, "HttpClientImpl::recvResponse");
    int state = 0;
    HttpClientImpl::Response response(*this);
    while (state < 40)
    {
        long timeout = remainingTime();
        if (timeout <= 0)
        {
            Logger::instance().warn("HttpClientImpl::recvResponse: Timed out.");
            break;
        }
        char tmp[256];
        ssize_t n = this->recv(tmp, sizeof(tmp));
        if (n > 0)
        {
            char* s1 = tmp;
            char* s2 = s1 + n;
            while (s1 < s2)
            {
                if (!ParseResponse(*s1 & 0xff, state, response))
                {
                    goto done;
                }
                s1++;
            }
        }
        else if (n < 0)
        {
            Logger::instance().error("CLI: Recv failed.");
            return false;
        }
        else
        {
            if (timeout > 100)
            {
                timeout = 100;
            }
            int rc = canRecv(timeout);
            if (rc < 0)
            {
                Logger::instance().warn("%s", _errbuf);
                break;
            }
        }
    }
done:
    TRACEPUT("state=%d", state);
    return state == 40;
}


const HttpClient::HeaderMap& HttpClientImpl::getResponseHeaderMap() const
{
    return _responseHeaderMap;
}


int HttpClientImpl::getResponseBodyLength() const
{
    return _responseBody.len();
}


int HttpClientImpl::getResponseBody(void* ptr, size_t len) const
{
    size_t lenReturn = _responseBody.len();
    if (lenReturn > len)
    {
        lenReturn = len;
    }
    memcpy(ptr, _responseBody.str(), lenReturn);
    return static_cast<int>(lenReturn);
}
