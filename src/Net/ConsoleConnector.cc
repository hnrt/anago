// Copyright (C) 2012-2017 Hideaki Narita


//#define NO_TRACE


#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include "Base/Atomic.h"
#include "Base/StringBuffer.h"
#include "Exception/ConsoleException.h"
#include "Logger/Trace.h"
#include "Protocol/HttpClient.h"
#include "ConsoleConnector.h"


using namespace hnrt;


#define IBUFSZ 16777216
#define OBUFSZ 65536


ConsoleConnector::ConsoleConnector()
    : _httpClient(HttpClient::create())
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
    TRACEFUN(this, "ConsoleConnector::open(%s,%s)", location, authorization);

    _sockHost = -1;
    _ibuf.clear();
    _obuf.clear();
    _statusCode = -1;
    _headerMap.clear();

    _httpClient->init();
    _httpClient->setUrl(location);
    _httpClient->setTcpNoDelay();
    if (!_httpClient->connect())
    {
        throw CommunicationConsoleException(_httpClient->getResult(), "%s", _httpClient->getError());
    }

    _sockHost = _httpClient->getSocket();
    if (_sockHost < 0)
    {
        throw CommunicationConsoleException(_httpClient->getResult(), "Socket unavailable.");
    }

    Glib::ustring request = getRequest(location, authorization);
    _obuf.write(request.c_str(), request.bytes());

    send();
}


void ConsoleConnector::close()
{
    TRACEFUN(this, "ConsoleConnector::close");

    _httpClient->fini();
}


ssize_t ConsoleConnector::recv()
{
    TRACEFUN(this, "ConsoleConnector::recv(pos=%zd,len=%zd)", _ibuf.wPos(), _ibuf.wLen());
    if (_ibuf.tryRewind())
    {
        TRACEPUT("Buffer rewinded.");
    }
    else if (!_ibuf.canWrite())
    {
        TRACEPUT("No space to receive.");
        return -1;
    }
    ssize_t n = _httpClient->recv(_ibuf.wPtr(), _ibuf.wLen());
    if (n > 0)
    {
        TRACEPUT("OK %zu", n);
        _ibuf.wPos(_ibuf.wPos() + n);
        return n;
    }
    else if (n == 0)
    {
        TRACEPUT("AGAIN (no data to receive)");
        return 0;
    }
    else
    {
        throw CommunicationConsoleException(_httpClient->getResult(), "Recv failed: %s", _httpClient->getError());
    }
}


ssize_t ConsoleConnector::send()
{
    TRACEFUN(this, "ConsoleConnector::send(pos=%zd,len=%zd)", _obuf.rPos(), _obuf.rLen());
    if (!_obuf.canRead())
    {
        TRACEPUT("No data to send.");
        return -1;
    }
    ssize_t n = _httpClient->send(_obuf.rPtr(), _obuf.rLen());
    if (n > 0)
    {
        TRACEPUT("OK %zu", n);
        _obuf.rPos(_obuf.rPos() + n);
        return n;
    }
    else if (n == 0)
    {
        TRACEPUT("AGAIN (busy)");
        return 0;
    }
    else
    {
        throw CommunicationConsoleException(_httpClient->getResult(), "Send failed: %s", _httpClient->getError());
    }
}


bool ConsoleConnector::canRecv(long timeoutInMicroseconds) const
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(_sockHost, &fds);
    struct timeval timeout;
    timeout.tv_sec = timeoutInMicroseconds / 1000000L;
    timeout.tv_usec = timeoutInMicroseconds % 1000000L;
    int rc = select(_sockHost + 1, &fds, NULL, NULL, &timeout);
    if (rc < 0)
    {
        int errorCode = errno;
        throw CommunicationConsoleException(errorCode + 65536, "select failed. error=%d (%s)", errorCode, strerror(errorCode));
    }
    return rc > 0;
}


bool ConsoleConnector::canSend(long timeoutInMicroseconds) const
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(_sockHost, &fds);
    struct timeval timeout;
    timeout.tv_sec = timeoutInMicroseconds / 1000000L;
    timeout.tv_usec = timeoutInMicroseconds % 1000000L;
    int rc = select(_sockHost + 1, NULL, &fds, NULL, &timeout);
    if (rc < 0)
    {
        int errorCode = errno;
        throw CommunicationConsoleException(errorCode + 65536, "select failed. error=%d (%s)", errorCode, strerror(errorCode));
    }
    return rc > 0;
}


Glib::ustring ConsoleConnector::getRequest(const char* location, const char* authorization)
{
    StringBuffer request;
    Glib::ustring host, absPathAndQuery;
    parseLocation(location, host, absPathAndQuery);
    request.format("CONNECT %s HTTP/1.0\r\n", absPathAndQuery.c_str());
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
    char* s9 = s0 + _ibuf.rLen();
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
