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


#define IBUFSZ 33554432
#define OBUFSZ 65536


ConsoleConnector::ConsoleConnector()
    : _curl(NULL)
    , _sockHost(-1)
    , _ibuf(IBUFSZ)
    , _obuf(OBUFSZ)
    , _statusCode(0)
{
}


ConsoleConnector::~ConsoleConnector()
{
}


void ConsoleConnector::open(const char* location, const char* authorization)
{
    TRACE(StringBuffer().format("ConsoleConnector@%zx::open", this), "location=%s authorization=%s", location, authorization);

    Glib::ustring request = getRequest(location, authorization);

    _obuf.clear();
    _obuf.put(request.c_str(), request.bytes());

    _sockHost = -1;
    _ibuf.clear();
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
    TRACE(StringBuffer().format("ConsoleConnector@%zx::close", this));

    CURL* curl = InterlockedExchangePointer(&_curl, reinterpret_cast<CURL*>(NULL));
    if (curl)
    {
        curl_easy_cleanup(curl);
    }
}


ssize_t ConsoleConnector::send()
{
    TRACE("ConsoleConnector::send", "pos=%zd len=%zd", _obuf.rPos(), _obuf.remaining());
    if (_obuf.remaining() <= 0)
    {
        TRACEPUT("No data to send.");
        return -1;
    }
    size_t n = 0;
    CURLcode result = curl_easy_send(_curl, _obuf.rPtr(), _obuf.remaining(), &n);
    if (result == CURLE_OK)
    {
        TRACEPUT("CURLE_OK %zu", n);
        _obuf.rPos(_obuf.rPos() + n);
        return n;
    }
    else if (result == CURLE_AGAIN)
    {
        TRACEPUT("CURLE_AGAIN (busy)");
        return 0;
    }
    else if (result == CURLE_UNSUPPORTED_PROTOCOL)
    {
        throw CommunicationConsoleException(result, "CURL: Send failed. error=UNSUPPORTED_PROTOCOL (possibly disconnected by host)");
    }
    else
    {
        throw CommunicationConsoleException(result, "CURL: Send failed. error=%d", result);
    }
}


ssize_t ConsoleConnector::recv()
{
    TRACE("ConsoleConnector::recv", "pos=%zd len=%zd", _ibuf.wPos(), _ibuf.space());
    if (_ibuf.space() <= 0)
    {
        TRACEPUT("No space to receive.");
        return -1;
    }
    size_t n = 0;
    CURLcode result = curl_easy_recv(_curl, _ibuf.wPtr(), _ibuf.space(), &n);
    _ibuf.wPos(_ibuf.wPos() + n);
    if (result == CURLE_OK)
    {
        TRACEPUT("CURLE_OK %zu", n);
        return n;
    }
    else if (result == CURLE_AGAIN)
    {
        TRACEPUT("CURLE_AGAIN (no data to receive)");
        return 0;
    }
    else if (result == CURLE_UNSUPPORTED_PROTOCOL)
    {
        throw CommunicationConsoleException(result, "CURL: Recv failed. error=UNSUPPORTED_PROTOCOL (possibly disconnected by host)");
    }
    else if (result == CURLE_BAD_FUNCTION_ARGUMENT)
    {
        throw CommunicationConsoleException(result, "CURL: Recv failed. error=BAD_FUNCTION_ARGUMENT");
    }
    else
    {
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


bool ConsoleConnector::getHeaderLength(size_t& length)
{
    char* s0 = (char*)_ibuf.rPtr();
    char* s1 = s0;
    char* s9 = s0 + _ibuf.remaining();
    while (1)
    {
        s1 = (char *)memchr(s1, '\r', s9 - s1);
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
            length = s1 - s0;
            return true;
        }
    }
}


bool ConsoleConnector::parseHeader(size_t length)
{
    char* s1 = (char*)_ibuf.rPtr();
    char* s9 = s1 + length;
    if (s1 + 5 <= s9 && !strncasecmp(s1, "HTTP/", 5))
    {
        s1 += 5;
    }
    else
    {
        return false;
    }
    char* s2 = s1;
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
    char* s3 = s2;
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
    s1 = (char*)memchr(s2, '\r', s9 - s2);
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
        char* s8 = (char*)memchr(s1, '\r', s9 - s1);
        if (s8 && s8 + 1 < s9 && *(s8 + 1) == '\n')
        {
            // ok
        }
        else
        {
            return false;
        }
        s2 = (char*)memchr(s1, ':', s8 - s1);
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
