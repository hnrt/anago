// Copyright (C) 2017 Hideaki Narita


// cf. https://github.com/xenserver/xenadmin/CommandLib/thinCLIProtocol.cs


#include <stdarg.h>
#include <sys/time.h>
#include <stdexcept>
#include "Base/StringBuffer.h"
#include "Base/Time.h"
#include "File/File.h"
#include "Logger/Trace.h"
#include "HttpClient.h"
#include "ThinClientInterfaceImpl.h"


using namespace hnrt;


ThinClientInterfaceImpl::ThinClientInterfaceImpl()
    : _timeoutInMilliseconds(0)
{
    Trace trace(this, "ThinClientInterfaceImpl::ctor");
}


ThinClientInterfaceImpl::~ThinClientInterfaceImpl()
{
    Trace trace(this, "ThinClientInterfaceImpl::dtor");
}


void ThinClientInterfaceImpl::init()
{
    Trace trace(this, "ThinClientInterfaceImpl::init");
}


void ThinClientInterfaceImpl::fini()
{
    Trace trace(this, "ThinClientInterfaceImpl::fini");
    _password.clear();
}


void ThinClientInterfaceImpl::setHostname(const char* hostname)
{
    Logger::instance().trace("ThinClientInterfaceImpl@%zx::setHostname(%s)", this, hostname);
    _hostname = hostname;
}


void ThinClientInterfaceImpl::setUsername(const char* username)
{
    Logger::instance().trace("ThinClientInterfaceImpl@%zx::setUsername(%s)", this, username);
    _username = username;
}


void ThinClientInterfaceImpl::setPassword(const char* password)
{
    Logger::instance().trace("ThinClientInterfaceImpl@%zx::setPassword(%s)", this, password);
    _password = password;
}


void ThinClientInterfaceImpl::setTimeout(long value)
{
    Logger::instance().trace("ThinClientInterfaceImpl@%zx::setTimeout(%ld)", this, value);
    _timeoutInMilliseconds = value;
}


void ThinClientInterfaceImpl::setPrintCallback(const sigc::slot<void, ThinClientInterface&>& printCb)
{
    _printCb = printCb;
}


void ThinClientInterfaceImpl::setPrintErrorCallback(const sigc::slot<void, ThinClientInterface&>& printErrorCb)
{
    _printErrorCb = printErrorCb;
}


void ThinClientInterfaceImpl::setExitCallback(const sigc::slot<void, ThinClientInterface&>& exitCb)
{
    _exitCb = exitCb;
}


void ThinClientInterfaceImpl::setProgressFunction(const ProgressFunction& function)
{
    _progressFunction = function;
}


void ThinClientInterfaceImpl::resetProgressFunction()
{
    _progressFunction.disconnect();
}


static bool Send(HttpClient& httpClient, const void* ptr, size_t len)
{
    const char* s1 = (const char*)ptr;
    const char* s2 = s1 + len;
    while (s1 < s2)
    {
        long timeout = httpClient.remainingTime();
        if (timeout <= 0)
        {
            Logger::instance().warn("CLI: Timed out.");
            return false;
        }
        ssize_t n = httpClient.send(s1, s2 - s1);
        if (n > 0)
        {
            Logger::instance().trace("httpClient.send: %'zu bytes", n);
            s1 += n;
        }
        else if (n < 0)
        {
            Logger::instance().error("CLI: Send failed.");
            return false;
        }
        else
        {
            if (timeout > 100)
            {
                timeout = 100;
            }
            int rc = httpClient.canSend(timeout);
            if (rc < 0)
            {
                Logger::instance().warn("CLI: %s", httpClient.getError());
                return false;
            }
        }
    }
    return true;
}


static bool Send(HttpClient& httpClient, int value)
{
    char buf[4];
    buf[0] = static_cast<char>((value >> (8 * 0)) & 0xff);
    buf[1] = static_cast<char>((value >> (8 * 1)) & 0xff);
    buf[2] = static_cast<char>((value >> (8 * 2)) & 0xff);
    buf[3] = static_cast<char>((value >> (8 * 3)) & 0xff);
    return Send(httpClient, buf, sizeof(buf));
}


static bool Send(HttpClient& httpClient, int value1, int value2)
{
    int length = static_cast<int>(sizeof(int) * 2);
    if (!Send(httpClient, length))
    {
        return false;
    }
    if (!Send(httpClient, value1))
    {
        return false;
    }
    if (!Send(httpClient, value2))
    {
        return false;
    }
    return true;
}


static bool Send(HttpClient& httpClient, int value1, int value2, int value3)
{
    int length = static_cast<int>(sizeof(int) * 3);
    if (!Send(httpClient, length))
    {
        return false;
    }
    if (!Send(httpClient, value1))
    {
        return false;
    }
    if (!Send(httpClient, value2))
    {
        return false;
    }
    if (!Send(httpClient, value3))
    {
        return false;
    }
    return true;
}


static bool Recv(HttpClient& httpClient, void* ptr, size_t len)
{
    char* r1 = (char*)ptr;
    char* r2 = r1 + len;
    while (r1 < r2)
    {
        long timeout = httpClient.remainingTime();
        if (timeout <= 0)
        {
            Logger::instance().warn("CLI: Timed out.");
            return false;
        }
        ssize_t n = httpClient.recv(r1, r2 - r1);
        if (n > 0)
        {
            Logger::instance().trace("httpClient.recv: %'zu bytes", n);
            r1 += n;
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
            int rc = httpClient.canRecv(timeout);
            if (rc < 0)
            {
                Logger::instance().warn("CLI: %s", httpClient.getError());
                return false;
            }
        }
    }
    return true;
}


static bool Recv(HttpClient& httpClient, int& value)
{
    unsigned char buf[4];
    if (!Recv(httpClient, buf, sizeof(buf)))
    {
        return false;
    }
    value =
        (static_cast<unsigned int>(buf[0]) << (8 * 0)) |
        (static_cast<unsigned int>(buf[1]) << (8 * 1)) |
        (static_cast<unsigned int>(buf[2]) << (8 * 2)) |
        (static_cast<unsigned int>(buf[3]) << (8 * 3));
    return true;
}


static bool Recv(HttpClient& httpClient, Glib::ustring& str)
{
    bool retval = false;
    char* buf = NULL;
    {
        int length;
        if (!Recv(httpClient, length))
        {
            goto done;
        }
        if (length < 0)
        {
            Logger::instance().error("CLI: Protocol error: Bad length (negative).");
            goto done;
        }
        if (length > 1048576)
        {
            Logger::instance().error("CLI: Protocol error: Bad length (too big).");
            goto done;
        }
        if (length == 0)
        {
            retval = true;
            goto done;
        }
        buf = new char[length];
        if (!Recv(httpClient, buf, length))
        {
            goto done;
        }
        char* p1 = buf;
        char* p2 = buf + length;
        while (p1 < p2)
        {
            if (*p1++ == '\0')
            {
                Logger::instance().warn("CLI: Protocol error: Bad string (null included).");
                *(p1 - 1) = ' ';
            }
        }
        str.assign(buf, length);
        retval = true;
    }
done:
    delete[] buf;
    return retval;
}


static bool Print(HttpClient& httpClient, Glib::ustring& output, Trace& trace)
{
    if (!Recv(httpClient, output))
    {
        return false;
    }

    trace.put("PRINT: %s", output.c_str());

    return true;
}


static bool PrintStderr(HttpClient& httpClient, Glib::ustring& output, Trace& trace)
{
    if (!Recv(httpClient, output))
    {
        return false;
    }

    trace.put("PRINT_STDERR: %s", output.c_str());

    return true;
}


static bool Debug(HttpClient& httpClient, Glib::ustring& output)
{
    if (!Recv(httpClient, output))
    {
        return false;
    }

    Logger::instance().debug("CLI: %s", output.c_str());

    return true;
}


static bool GetError(HttpClient& httpClient, Glib::ustring& output)
{
    if (!Recv(httpClient, output))
    {
        return false;
    }

    int count;

    if (!Recv(httpClient, count))
    {
        return false;
    }

    while (count-- > 0)
    {
        Glib::ustring message;
        if (!Recv(httpClient, message))
        {
            return false;
        }
        output.append("\n");
        output.append(message);
    }

    Logger::instance().error("CLI: %s", output.c_str());

    return true;
}


static bool Load(HttpClient& httpClient, char* buf, size_t bufsz, Trace& trace)
{
    {
        Glib::ustring filename;

        if (!Recv(httpClient, filename))
        {
            goto error;
        }

        trace.put("LOAD: filename=%s", filename.c_str());

        RefPtr<File> file = File::create(filename.c_str(), "r");

        if (!file->open())
        {
            Logger::instance().error("%s: %s", file->path(), strerror(file->error()));
            goto error;
        }

        if (!Send(httpClient, ThinClientInterfaceImpl::RESPONSE, ThinClientInterfaceImpl::OK))
        {
            return false;
        }

        if (!Send(httpClient, ThinClientInterfaceImpl::BLOB, ThinClientInterfaceImpl::CHUNK, (int)file->size()))
        {
            return false;
        }

        while (1)
        {
            size_t n = file->read(buf, ThinClientInterfaceImpl::BLOCK_SIZE);

            if (n == 0)
            {
                break;
            }

            if (!Send(httpClient, buf, n))
            {
                return false;
            }
        }

        return Send(httpClient, ThinClientInterfaceImpl::BLOB, ThinClientInterfaceImpl::END);
    }

error:

    Send(httpClient, ThinClientInterfaceImpl::RESPONSE, ThinClientInterfaceImpl::FAILED);

    return false;
}


static bool Connect(HttpClient& httpClient, Glib::ustring& url, const char* buf, Trace& trace)
{
    while (1)
    {
        httpClient.init();
        httpClient.setFreshConnect();
        httpClient.setUrl(url.c_str());
        httpClient.setVerbose(true);

        if (!httpClient.connect())
        {
            Logger::instance().error("CLI: Connect(2) failed.");
            return false;
        }

        if (!Send(httpClient, buf, strlen(buf)))
        {
            return false;
        }

        if (!httpClient.recvResponse())
        {
            return false;
        }

        int status = httpClient.getStatus();

        trace.put("status=%d", status);

        if (status == 200)
        {
            return true;
        }
        else if (status == 302)
        {
            const HttpClient::HeaderMap& map = httpClient.getResponseHeaderMap();
            HttpClient::HeaderMap::const_iterator iter = map.find("location");
            if (iter == map.end())
            {
                Logger::instance().error("CLI: Missing location header.");
                return false;
            }
            url = iter->second.value;
            trace.put("Location: %s\n", url.c_str());
            httpClient.fini();
        }
        else
        {
            return false;
        }
    }
}


static bool HttpPut(HttpClient& httpClient, const Glib::ustring& hostname, char* buf, size_t bufsz, ThinClientInterface::ProgressFunction& report, Trace& trace)
{
    {
        Glib::ustring filename;

        if (!Recv(httpClient, filename))
        {
            goto error;
        }

        trace.put("HTTP_PUT: filename=%s", filename.c_str());

        Glib::ustring path;

        if (!Recv(httpClient, path))
        {
            goto error;
        }

        trace.put("HTTP_PUT: path=%s", path.c_str());

        RefPtr<File> file = File::create(filename.c_str(), "r");

        if (!file->open())
        {
            Logger::instance().error("%s: %s", file->path(), strerror(file->error()));
            goto error;
        }

        RefPtr<HttpClient> httpClient2 = HttpClient::create();

        Glib::ustring url = Glib::ustring::compose("https://%1%2", hostname, path);

        snprintf(buf, bufsz,
                 "PUT %s HTTP/1.0\r\n"
                 "Content-Length: %zu\r\n"
                 "\r\n",
                 path.c_str(),
                 file->size());

        if (!Connect(*httpClient2, url, buf, trace))
        {
            goto error;
        }

        while (1)
        {
            size_t n = file->read(buf, bufsz);

            if (n == 0)
            {
                break;
            }

            if (!Send(*httpClient2, buf, n))
            {
                goto error;
            }

            if (!report.empty())
            {
                report(file->nbytes(), file->size(), file->path());
            }
        }

        return Send(httpClient, ThinClientInterfaceImpl::RESPONSE, ThinClientInterfaceImpl::OK);
    }

error:

    Send(httpClient, ThinClientInterfaceImpl::RESPONSE, ThinClientInterfaceImpl::FAILED);

    return false;
}


static bool HttpGet(HttpClient& httpClient, const Glib::ustring& hostname, char* buf, size_t bufsz, Trace& trace)
{
    {
        Glib::ustring filename;

        if (!Recv(httpClient, filename))
        {
            goto error;
        }

        trace.put("HTTP_GET: filename=%s", filename.c_str());

        Glib::ustring path;

        if (!Recv(httpClient, path))
        {
            goto error;
        }

        trace.put("HTTP_GET: path=%s", path.c_str());

        RefPtr<File> file = File::create(filename.c_str(), "w");

        if (file->exists())
        {
            Logger::instance().error("%s: Already exists.", file->path());
            goto error;
        }

        if (!file->open())
        {
            Logger::instance().error("%s: %s", file->path(), strerror(file->error()));
            goto error;
        }

        RefPtr<HttpClient> httpClient2 = HttpClient::create();

        Glib::ustring url = Glib::ustring::compose("https://%1%2", hostname, path);

        snprintf(buf, bufsz,
                 "GET %s HTTP/1.0\r\n"
                 "\r\n",
                 path.c_str());

        if (!Connect(*httpClient2, url, buf, trace))
        {
            goto error;
        }

        while (1)
        {
            ssize_t n = httpClient2->recv(buf, bufsz);

            if (n == 0)
            {
                break;
            }
            else if (n > 0)
            {
                file->write(buf, n);
            }
            else
            {
                goto error;
            }
        }

        return Send(httpClient, ThinClientInterfaceImpl::RESPONSE, ThinClientInterfaceImpl::OK);
    }

error:

    Send(httpClient, ThinClientInterfaceImpl::RESPONSE, ThinClientInterfaceImpl::FAILED);

    return false;
}


bool ThinClientInterfaceImpl::run(const char* arg, ...)
{
    Trace trace(this, "ThinClientInterfaceImpl::run");

    if (!arg)
    {
        throw std::invalid_argument("ThinClientInterfaceImpl::run: Command is empty.");
    }

    bool retval = false;
    char* buf = new char[BLOCK_SIZE];

    Time expiry;
    expiry.addMilliseconds(_timeoutInMilliseconds);

    {
        RefPtr<HttpClient> httpClient = HttpClient::create();

        Glib::ustring url = Glib::ustring::compose("https://%1/cli", _hostname);

        StringBuffer body;

        va_list argList;
        va_start(argList, arg);
        do
        {
            trace.put("arg=%s", arg);
            body.appendFormat("%s\n", arg);
            arg = va_arg(argList, const char*);
        }
        while (arg);
        va_end(argList);
        body.appendFormat("username=%s\n", _username.c_str());
        body.appendFormat("password=%s", _password.c_str());

        snprintf(buf, BLOCK_SIZE,
                 "POST /cli HTTP/1.0\r\n"
                 "content-length: %d\r\n"
                 "\r\n"
                 "%s",
                 body.len(),
                 body.str());

        httpClient->init();
        httpClient->setTimeout(_timeoutInMilliseconds);
        httpClient->setFreshConnect();
        httpClient->setUrl(url.c_str());
        httpClient->setVerbose(true);

        if (!httpClient->connect())
        {
            Logger::instance().error("CLI: Connect failed.");
            goto done;
        }

        if (!Send(*httpClient, buf, strlen(buf)))
        {
            goto done;
        }

        static const char magic[] = { "XenSource thin CLI protocol" };
        size_t magicLen = strlen(magic);

        int major;
        int minor;

        if (!Recv(*httpClient, buf, magicLen))
        {
            goto done;
        }
        if (memcmp(buf, magic, magicLen))
        {
            Logger::instance().error("CLI: Magic mismatch.");
            goto done;
        }
        if (!Recv(*httpClient, major))
        {
            goto done;
        }
        if (!Recv(*httpClient, minor))
        {
            goto done;
        }

        trace.put("Version=%d.%d", major, minor);

        major = VERSION_MAJOR;
        minor = VERSION_MINOR;

        if (!Send(*httpClient, magic, magicLen))
        {
            goto done;
        }
        if (!Send(*httpClient, major))
        {
            goto done;
        }
        if (!Send(*httpClient, minor))
        {
            goto done;
        }

        bool next = true;
        while (next)
        {
            int length;
            int command1;
            int command2;

            if (!Recv(*httpClient, length))
            {
                goto done;
            }

            if (!Recv(*httpClient, command1))
            {
                goto done;
            }

            if (command1 != COMMAND)
            {
                Logger::instance().error("CLI: Unknown command: %d", command1);
                goto done;
            }

            if (!Recv(*httpClient, command2))
            {
                goto done;
            }

            switch (command2)
            {
            case PRINT:
                if (Print(*httpClient, _output, trace))
                {
                    if (!_printCb.empty())
                    {
                        _printCb(*this);
                    }
                }
                else
                {
                    goto done;
                }
                break;

            case PRINT_STDERR:
                if (PrintStderr(*httpClient, _errorOutput, trace))
                {
                    if (!_printErrorCb.empty())
                    {
                        _printErrorCb(*this);
                    }
                }
                else
                {
                    goto done;
                }
                break;

            case DEBUG:
                if (!Debug(*httpClient, _output))
                {
                    goto done;
                }
                break;

            case EXIT:
                if (Recv(*httpClient, _exitCode))
                {
                    trace.put("EXIT: %d", _exitCode);
                    next = false;
                    if (!_exitCb.empty())
                    {
                        _exitCb(*this);
                    }
                }
                else
                {
                    goto done;
                }
                break;

            case ERROR:
                if (!GetError(*httpClient, _errorOutput))
                {
                    goto done;
                }
                break;

            case PROMPT:
                Logger::instance().error("CLI: PROMPT not implemented.");
                goto done;

            case LOAD:
                if (Load(*httpClient, buf, BLOCK_SIZE, trace))
                {
                    goto done;
                }
                break;

            case HTTP_PUT:
                if (!HttpPut(*httpClient, _hostname, buf, BLOCK_SIZE, _progressFunction, trace))
                {
                    goto done;
                }
                break;

            case HTTP_GET:
                if (!HttpGet(*httpClient, _hostname, buf, BLOCK_SIZE, trace))
                {
                    goto done;
                }
                break;

            default:
                Logger::instance().error("CLI: Unknown sub-command: %d", command2);
                goto done;
            }
        }

        retval = true;
    }

done:

    delete[] buf;

    return retval;
}
