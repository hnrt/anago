// Copyright (C) 2017 Hideaki Narita


// cf. https://github.com/xenserver/xenadmin/CommandLib/thinCLIProtocol.cs


#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdexcept>
#include "Base/StringBuffer.h"
#include "Base/Time.h"
#include "File/File.h"
#include "Logger/Trace.h"
#include "HttpClient.h"
#include "ThinClientInterfaceImpl.h"


using namespace hnrt;


#ifdef _DEBUG
#define VERBOSE_OUTPUT true
#else
#define VERBOSE_OUTPUT false
#endif


ThinClientInterfaceImpl::CommandFunctionMap ThinClientInterfaceImpl::_commandFunctionMap;


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
    if (_commandFunctionMap.empty())
    {
        _commandFunctionMap.insert(CommandFunctionMapEntry(PRINT, &ThinClientInterfaceImpl::print));
        _commandFunctionMap.insert(CommandFunctionMapEntry(PRINT_STDERR, &ThinClientInterfaceImpl::printStderr));
        _commandFunctionMap.insert(CommandFunctionMapEntry(DEBUG, &ThinClientInterfaceImpl::debug));
        _commandFunctionMap.insert(CommandFunctionMapEntry(EXIT, &ThinClientInterfaceImpl::exit));
        _commandFunctionMap.insert(CommandFunctionMapEntry(ERROR, &ThinClientInterfaceImpl::error));
        _commandFunctionMap.insert(CommandFunctionMapEntry(PROMPT, &ThinClientInterfaceImpl::prompt));
        _commandFunctionMap.insert(CommandFunctionMapEntry(LOAD, &ThinClientInterfaceImpl::load));
        _commandFunctionMap.insert(CommandFunctionMapEntry(HTTP_PUT, &ThinClientInterfaceImpl::httpPut));
        _commandFunctionMap.insert(CommandFunctionMapEntry(HTTP_GET, &ThinClientInterfaceImpl::httpGet));
    }
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


void ThinClientInterfaceImpl::setPrintFunction(const PrintFunction& function)
{
    _printFunction = function;
}


void ThinClientInterfaceImpl::resetPrintFunction()
{
    _printFunction.disconnect();
}


void ThinClientInterfaceImpl::setPrintErrorFunction(const PrintFunction& function)
{
    _printErrorFunction = function;
}


void ThinClientInterfaceImpl::resetPrintErrorFunction()
{
    _printErrorFunction.disconnect();
}


void ThinClientInterfaceImpl::setExitFunction(const ExitFunction& function)
{
    _exitFunction = function;
}


void ThinClientInterfaceImpl::resetExitFunction()
{
    _exitFunction.disconnect();
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


bool ThinClientInterfaceImpl::run(const char* arg, ...)
{
    Trace trace(this, "ThinClientInterfaceImpl::run");

    if (!arg)
    {
        throw std::invalid_argument("ThinClientInterfaceImpl::run: Command is empty.");
    }

    bool retval = false;

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

        snprintf(_buf, BLOCK_SIZE,
                 "POST /cli HTTP/1.0\r\n"
                 "content-length: %d\r\n"
                 "\r\n"
                 "%s",
                 body.len(),
                 body.str());

        httpClient->init();
        httpClient->setTimeout(_timeoutInMilliseconds);
        httpClient->setUrl(url.c_str());
        httpClient->setVerbose(VERBOSE_OUTPUT);

        if (!httpClient->connect())
        {
            Logger::instance().error("CLI: Connect failed.");
            goto done;
        }

        if (!Send(*httpClient, _buf, strlen(_buf)))
        {
            goto done;
        }

        static const char magic[] = { "XenSource thin CLI protocol" };
        size_t magicLen = strlen(magic);

        int major;
        int minor;

        if (!Recv(*httpClient, _buf, magicLen))
        {
            goto done;
        }
        if (memcmp(_buf, magic, magicLen))
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

        do
        {
            int length;
            int type;

            _command = -1;
            _contentLength = static_cast<size_t>(-1);

            if (!Recv(*httpClient, length))
            {
                goto done;
            }

            if (!Recv(*httpClient, type))
            {
                goto done;
            }

            if (type != COMMAND)
            {
                Logger::instance().error("CLI: Unknown type: %d", type);
                goto done;
            }

            if (!Recv(*httpClient, _command))
            {
                goto done;
            }

            CommandFunctionMap::const_iterator iter = _commandFunctionMap.find(_command);
            if (iter == _commandFunctionMap.end())
            {
                Logger::instance().error("CLI: Unknown command: %d", _command);
                goto done;
            }

            const CommandFunction& function = iter->second;
            if (!(this->*function)(*httpClient))
            {
                goto done;
            }
        }
        while (_command != EXIT);

        retval = true;
    }

done:

    return retval;
}


bool ThinClientInterfaceImpl::print(HttpClient& httpClient)
{
    Trace trace(this, "ThinClientInterfaceImpl::print");

    Glib::ustring output;

    if (!Recv(httpClient, output))
    {
        return false;
    }

    trace.put("%s", output.c_str());

    if (!_printFunction.empty())
    {
        _printFunction(output.c_str());
    }

    return true;
}


bool ThinClientInterfaceImpl::printStderr(HttpClient& httpClient)
{
    Trace trace(this, "ThinClientInterfaceImpl::printStderr");

    Glib::ustring output;

    if (!Recv(httpClient, output))
    {
        return false;
    }

    trace.put("%s", output.c_str());

    if (!_printErrorFunction.empty())
    {
        _printErrorFunction(output.c_str());
    }

    return true;
}


bool ThinClientInterfaceImpl::debug(HttpClient& httpClient)
{
    Trace trace(this, "ThinClientInterfaceImpl::debug");

    Glib::ustring output;

    if (!Recv(httpClient, output))
    {
        return false;
    }

    Logger::instance().debug("CLI: %s", output.c_str());

    return true;
}


bool ThinClientInterfaceImpl::exit(HttpClient& httpClient)
{
    Trace trace(this, "ThinClientInterfaceImpl::exit");

    int exitCode;

    if (!Recv(httpClient, exitCode))
    {
        return false;
    }

    trace.put("exitCode=%d", exitCode);

    if (!_exitFunction.empty())
    {
        _exitFunction(exitCode);
    }

    return true;
}


bool ThinClientInterfaceImpl::error(HttpClient& httpClient)
{
    Trace trace(this, "ThinClientInterfaceImpl::error");

    Glib::ustring output;

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

    if (!_printErrorFunction.empty())
    {
        _printErrorFunction(output.c_str());
    }

    return true;
}


bool ThinClientInterfaceImpl::prompt(HttpClient& httpClient)
{
    Logger::instance().error("CLI: PROMPT not implemented.");
    return false;
}


bool ThinClientInterfaceImpl::load(HttpClient& httpClient)
{
    Trace trace(this, "ThinClientInterfaceImpl::load");

    {
        Glib::ustring filename;

        if (!Recv(httpClient, filename))
        {
            goto error;
        }

        trace.put("filename=%s", filename.c_str());

        RefPtr<File> file = File::create(filename.c_str(), "r");

        if (!file->open())
        {
            Logger::instance().error("%s: %s", file->path(), strerror(file->error()));
            goto error;
        }

        _contentLength = file->size();

        if (!Send(httpClient, RESPONSE, OK))
        {
            return false;
        }

        if (!Send(httpClient, BLOB, CHUNK, (int)file->size()))
        {
            return false;
        }

        while (1)
        {
            size_t n = file->read(_buf, BLOCK_SIZE);

            if (n == 0)
            {
                break;
            }

            if (!Send(httpClient, _buf, n))
            {
                return false;
            }

            if (!_progressFunction.empty())
            {
                _progressFunction(file->nbytes());
            }
        }

        return Send(httpClient, BLOB, END);
    }

error:

    Send(httpClient, RESPONSE, FAILED);

    return false;
}


bool ThinClientInterfaceImpl::httpPut(HttpClient& httpClient)
{
    Trace trace(this, "ThinClientInterfaceImpl::httpPut");

    {
        Glib::ustring filename;

        if (!Recv(httpClient, filename))
        {
            goto error;
        }

        trace.put("filename=%s", filename.c_str());

        Glib::ustring path;

        if (!Recv(httpClient, path))
        {
            goto error;
        }

        trace.put("path=%s", path.c_str());

        RefPtr<File> file = File::create(filename.c_str(), "r");

        if (!file->open())
        {
            Logger::instance().error("%s: %s", file->path(), strerror(file->error()));
            goto error;
        }

        RefPtr<HttpClient> httpClient2 = HttpClient::create();

        Glib::ustring url = Glib::ustring::compose("https://%1%2", _hostname, path);

        snprintf(_buf, BLOCK_SIZE,
                 "PUT %s HTTP/1.0\r\n"
                 "Content-Length: %zu\r\n"
                 "\r\n",
                 path.c_str(),
                 file->size());

        if (!connect(*httpClient2, url, _buf))
        {
            goto error;
        }

        while (1)
        {
            size_t n = file->read(_buf, BLOCK_SIZE);

            if (n == 0)
            {
                break;
            }

            if (!Send(*httpClient2, _buf, n))
            {
                goto error;
            }

            if (!_progressFunction.empty())
            {
                _progressFunction(file->nbytes());
            }
        }

        return Send(httpClient, RESPONSE, OK);
    }

error:

    Send(httpClient, RESPONSE, FAILED);

    return false;
}


bool ThinClientInterfaceImpl::httpGet(HttpClient& httpClient)
{
    Trace trace(this, "ThinClientInterfaceImpl::httpGet");

    {
        Glib::ustring filename;

        if (!Recv(httpClient, filename))
        {
            goto error;
        }

        trace.put("filename=%s", filename.c_str());

        Glib::ustring path;

        if (!Recv(httpClient, path))
        {
            goto error;
        }

        trace.put("path=%s", path.c_str());

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

        Glib::ustring url = Glib::ustring::compose("https://%1%2", _hostname, path);

        snprintf(_buf, BLOCK_SIZE,
                 "GET %s HTTP/1.0\r\n"
                 "\r\n",
                 path.c_str());

        if (!connect(*httpClient2, url, _buf))
        {
            goto error;
        }

        while (1)
        {
            ssize_t n = httpClient2->recv(_buf, BLOCK_SIZE);

            if (n == 0)
            {
                if (_contentLength != static_cast<size_t>(-1) &&
                    file->nbytes() < _contentLength &&
                    httpClient2->canRecv(100) >= 0)
                {
                    continue;
                }
                break;
            }
            else if (n > 0)
            {
                file->write(_buf, n);
            }
            else
            {
                goto error;
            }

            if (!_progressFunction.empty())
            {
                _progressFunction(file->nbytes());
            }
        }

        return Send(httpClient, RESPONSE, OK);
    }

error:

    Send(httpClient, RESPONSE, FAILED);

    return false;
}


bool ThinClientInterfaceImpl::connect(HttpClient& httpClient, Glib::ustring& url, const char* request)
{
    Trace trace(this, "ThinClientInterfaceImpl::connect");

    while (1)
    {
        trace.put("url=%s", url.c_str());

        httpClient.init();
        httpClient.setUrl(url.c_str());
        httpClient.setVerbose(VERBOSE_OUTPUT);

        if (!httpClient.connect())
        {
            Logger::instance().error("CLI: Connect(2) failed.");
            return false;
        }

        if (!Send(httpClient, request, strlen(request)))
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
            const HttpClient::HeaderMap& map = httpClient.getResponseHeaderMap();
            HttpClient::HeaderMap::const_iterator iter = map.find("content-length");
            if (iter != map.end())
            {
                _contentLength = strtoul(iter->second.value.c_str(), NULL, 10);
                trace.put("content-length=%zu", _contentLength);
            }
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
            httpClient.fini();
        }
        else
        {
            return false;
        }
    }
}
