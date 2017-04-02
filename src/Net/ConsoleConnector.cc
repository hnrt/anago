// Copyright (C) 2012-2017 Hideaki Narita


#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdexcept>
#include "Base/Atomic.h"
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Exception/ConsoleException.h"
#include "ConsoleConnector.h"


using namespace hnrt;


#define BUFSZ 32768


ConsoleConnector::ConsoleConnector()
    : _curl(NULL)
    , _sockHost(-1)
    , _ibuf(new Buffer())
    , _obuf(NULL)
    , _statusCode(0)
{
}


ConsoleConnector::~ConsoleConnector()
{
    clear();
    delete _ibuf;
}


void ConsoleConnector::open(const char* location, const char* authorization)
{
    TRACE("ConsoleConnector::open", "location=%s authorization=%s", location, authorization);

    clear();

    Glib::ustring request = getRequest(location, authorization);
    enqueue(new Buffer(request.c_str(), request.bytes()));

    _sockHost = -1;
    _ibuf->extend(BUFSZ);
    _ibuf->len = 0;
    _statusCode = -1;
    _headerMap.clear();

    _curl = curl_easy_init();
    if (!_curl)
    {
        throw CommunicationConsoleException(CURLE_FAILED_INIT, "CURL unavailable.");
    }
    //curl_easy_setopt(_curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(_curl, CURLOPT_URL, location);
    curl_easy_setopt(_curl, CURLOPT_CONNECT_ONLY, 1);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(_curl, CURLOPT_TCP_NODELAY, 1);

    CURLcode result = curl_easy_perform(_curl);
    if (result != CURLE_OK)
    {
        throw CommunicationConsoleException(result, "CURL failed. error=%d", result);
    }

    long lastSocket = -1;
    curl_easy_getinfo(_curl, CURLINFO_LASTSOCKET, &lastSocket);
    if (lastSocket < 0)
    {
        throw CommunicationConsoleException(CURLE_FAILED_INIT, "CURL: Socket unavailable.");
    }
    if (lastSocket > INT_MAX)
    {
        throw CommunicationConsoleException(CURLE_FAILED_INIT, "CURL: Invalid socket value.");
    }
    _sockHost = (int)lastSocket;

    send();
}


void ConsoleConnector::close()
{
    CURL* curl = InterlockedExchangePointer(&_curl, reinterpret_cast<CURL*>(NULL));
    if (curl)
    {
        curl_easy_cleanup(curl);
    }
}


void ConsoleConnector::clear()
{
    Glib::Mutex::Lock lock(_omutex);
    delete _obuf;
    while (!_oqueue.empty())
    {
        _obuf = _oqueue.front();
        _oqueue.pop_front();
        delete _obuf;
    }
    _obuf = NULL;
}


void ConsoleConnector::enqueue(Buffer* buf)
{
    Glib::Mutex::Lock lock(_omutex);
    _oqueue.push_back(buf);
}


bool ConsoleConnector::dequeue()
{
    Glib::Mutex::Lock lock(_omutex);
    if (_oqueue.empty())
    {
        return false;
    }
    else
    {
        delete _obuf;
        _obuf = _oqueue.front();
        _oqueue.pop_front();
        return true;
    }
}


ssize_t ConsoleConnector::send()
{
    if (!_obuf || _obuf->size <= _obuf->len)
    {
        if (!dequeue())
        {
            // no data
            return -1;
        }
    }
    size_t n = 0;
    CURLcode result = curl_easy_send(_curl, _obuf->cur(), _obuf->curLen(), &n);
    if (result == CURLE_OK)
    {
        Logger::instance().trace("ConsoleConnector::send: CURLE_OK %zu", n);
        _obuf->len += n;
        return n;
    }
    else if (result == CURLE_AGAIN)
    {
        Logger::instance().trace("ConsoleConnector::send: CURLE_AGAIN");
        // ok to continue
        return 0;
    }
    else if (result == CURLE_UNSUPPORTED_PROTOCOL)
    {
        Logger::instance().trace("ConsoleConnector::send: CURLE_UNSUPPORTED_PROTOCOL");
        // possibly disconnected by host
        throw CommunicationConsoleException(result, "CURL: Send failed. error=UNSUPPORTED_PROTOCOL");
    }
    else
    {
        Logger::instance().trace("ConsoleConnector::send: CURLE_%d", result);
        throw CommunicationConsoleException(result, "CURL: Send failed. error=%d", result);
    }
}


size_t ConsoleConnector::recv()
{
    _ibuf->extend(_ibuf->len + 256);
    Logger::instance().trace("ConsoleConnector::recv: %zx %zu", _ibuf->cur(), _ibuf->curLen());
    size_t n = 0;
    CURLcode result = curl_easy_recv(_curl, _ibuf->cur(), _ibuf->curLen(), &n);
    if (result == CURLE_OK)
    {
        Logger::instance().trace("ConsoleConnector::recv: CURLE_OK %zu", n);
        _ibuf->len += n;
        return n;
    }
    else if (result == CURLE_AGAIN)
    {
        Logger::instance().trace("ConsoleConnector::recv: CURLE_AGAIN");
        // ok to continue
        return 0;
    }
    else if (result == CURLE_UNSUPPORTED_PROTOCOL)
    {
        Logger::instance().trace("ConsoleConnector::recv: CURLE_UNSUPPORTED_PROTOCOL");
        // possibly disconnected by host
        throw CommunicationConsoleException(result, "CURL: Recv failed. error=UNSUPPORTED_PROTOCOL");
    }
    else if (result == CURLE_BAD_FUNCTION_ARGUMENT)
    {
        Logger::instance().trace("ConsoleConnector::recv: CURLE_BAD_FUNCTION_ARGUMENT");
        throw CommunicationConsoleException(result, "CURL: Recv failed. error=BAD_FUNCTION_ARGUMENT");
    }
    else
    {
        Logger::instance().trace("ConsoleConnector::recv: CURLE_%d", result);
        throw CommunicationConsoleException(result, "CURL: Recv failed. error=%d", result);
    }
}


Glib::ustring ConsoleConnector::getRequest(const char* location, const char* authorization)
{
    StringBuffer request;
    Glib::ustring host, absPathAndQuery;
    parseLocation(location, host, absPathAndQuery);
    request.format("CONNECT %s HTTP/1.1\r\n", absPathAndQuery.c_str());
    request.appendFormat("Host: %s\r\n", host.c_str());
    request.appendFormat("Authorization: Basic %s\r\n", authorization);
    request.appendFormat("Accept: */*\r\n\r\n");
    return Glib::ustring(request);
}


void ConsoleConnector::parseLocation(const char* location, Glib::ustring& host, Glib::ustring& absPathAndQuery)
{
    const char* s1 = location ? strchr(location, ':') : NULL;
    if (s1)
    {
        s1++;
    }
    else
    {
    failure:
        throw LocationConsoleException(gettext("Malformed location: %s"), location ? location : "");
    }
    if (*s1 == '/')
    {
        s1++;
    }
    else
    {
        goto failure;
    }
    if (*s1 == '/')
    {
        s1++;
    }
    else
    {
        goto failure;
    }
    const char* s2 = strchr(s1, '/');
    if (s2)
    {
        host.assign(s1, s2 - s1);
        absPathAndQuery.assign(s2);
    }
    else
    {
        goto failure;
    }
}


bool ConsoleConnector::getHeaderLength(const char* s, size_t n, size_t& length)
{
    const char* s1 = s;
    const char* s9 = s + n;
    while (1)
    {
        s1 = (const char *)memchr(s1, '\r', s9 - s1);
        if (s1)
        {
            s1++;
        }
        else
        {
            return false;
        }
        if (s1 + 3 > s9)
        {
            return false;
        }
        else if (!memcmp(s1, "\n\r\n", 3))
        {
            s1 += 3;
            length = s1 - s;
            return true;
        }
    }
}


bool ConsoleConnector::parseHeader(const char* s, size_t n)
{
    const char* s1 = s;
    const char* s9 = s + n;
    if (s1 + 5 <= s9 && !strncasecmp(s1, "HTTP/", 5))
    {
        s1 += 5;
    }
    else
    {
        return false;
    }
    const char* s2 = s1;
    if (s2 < s9 && isdigit(*s2))
    {
        s2++;
    }
    else
    {
        return false;
    }
    while (s2 < s9 && isdigit(*s2))
    {
        s2++;
    }
    if (s2 < s9 && *s2 == '.')
    {
        s2++;
    }
    else
    {
        return false;
    }
    const char* s3 = s2;
    if (s3 < s9 && isdigit(*s3))
    {
        s3++;
    }
    else
    {
        return false;
    }
    while (s3 < s9 && isdigit(*s3))
    {
        s3++;
    }
    if (s3 < s9 && *s3 == ' ')
    {
        s3++;
    }
    else
    {
        return false;
    }
    unsigned long verMajor = strtoul(s1, NULL, 10);
    unsigned long verMinor = strtoul(s2, NULL, 10);
    (void)verMajor;
    (void)verMinor;
    s1 = s2 = s3;
    if (s2 < s9 && isdigit(*s2))
    {
        s2++;
    }
    else
    {
        return false;
    }
    while (s2 < s9 && isdigit(*s2))
    {
        s2++;
    }
    if (s2 < s9 && *s2 == ' ')
    {
        s2++;
    }
    else
    {
        return false;
    }
    _statusCode = (int)strtoul(s1, NULL, 10);
    s1 = (const char*)memchr(s2, '\r', s9 - s2);
    if (s1)
    {
        s1++;
    }
    else
    {
        return false;
    }
    if (s1 < s9 && *s1 == '\n')
    {
        s1++;
    }
    else
    {
        return false;
    }
    while (s1 < s9)
    {
        if (s1 < s9 && *s1 == '\r')
        {
            s1++;
            if (s1 + 1 == s9 && *s1 == '\n')
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        const char* s8 = (const char*)memchr(s1, '\r', s9 - s1);
        if (s8 && s8 + 1 < s9 && *(s8 + 1) == '\n')
        {
            // ok
        }
        else
        {
            return false;
        }
        s2 = (const char*)memchr(s1, ':', s8 - s1);
        if (!s2)
        {
            return false;
        }
        Glib::ustring name(s1, s2 - s1);
        s1 = s2 + 1;
        while (s1 < s8 && *s1 == ' ')
        {
            s1++;
        }
        Glib::ustring value(s1, s8 - s1);
        _headerMap.insert(HeaderEntry(name, value));
        s1 = s8 + 2;
    }
    return false;
}


ConsoleConnector::Buffer::Buffer(size_t fixedSize)
    : addr(NULL)
    , size(0)
    , len(0)
    , fixed(false)
{
    if (fixedSize)
    {
        fixed = true;
        addr = (char*)malloc(fixedSize);
        if (!addr)
        {
            throw std::bad_alloc();
        }
        size = fixedSize;
    }
}


ConsoleConnector::Buffer::Buffer(const void* p, size_t n)
    : addr(NULL)
    , size(0)
    , len(0)
    , fixed(false)
{
    if (p && n)
    {
        fixed = true;
        addr = (char*)malloc(n);
        if (!addr)
        {
            throw std::bad_alloc();
        }
        memcpy(addr, p, n);
        size = n;
        //Logger::instance().trace("ConsoleConnector::Buffer::ctor: %zu [%.*s]", n, (int)n, (char*)addr);
    }
}


ConsoleConnector::Buffer::~Buffer()
{
    free(addr);
}


bool ConsoleConnector::Buffer::extend(size_t sizeHint)
{
    if (fixed)
    {
        throw std::runtime_error("ConsoleConnector::Buffer::extend: Fixed buffer.");
    }
    if (0 < sizeHint && sizeHint <= size)
    {
        return false;
    }
    size_t sizeNew = size;
    do
    {
        sizeNew += BUFSZ;
    }
    while (sizeNew < sizeHint);
    char* addrNew = (char*)realloc(addr, sizeNew);
    if (!addrNew)
    {
        throw std::bad_alloc();
    }
    addr = addrNew;
    size = sizeNew;
    return true;
}


void ConsoleConnector::Buffer::append(const void* p, size_t n)
{
    if (p && n)
    {
        if (size < len + n)
        {
            extend(len + n);
        }
        memcpy(addr + len, p, n);
        len += n;
    }
}


void ConsoleConnector::Buffer::discard(size_t n)
{
    if (len <= n)
    {
        len = 0;
    }
    else
    {
        len -= n;
        memmove(addr, addr + n, len);
    }
}
