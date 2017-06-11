// Copyright (C) 2012-2017 Hideaki Narita


#include "Base/StringBuffer.h"
#include "File/File.h"
#include "Logger/Trace.h"
#include "Protocol/HttpClient.h"
#include "XenServer/Session.h"
#include "PatchUploader.h"


using namespace hnrt;


RefPtr<PatchUploader> PatchUploader::create()
{
    return RefPtr<PatchUploader>(new PatchUploader);
}


PatchUploader::PatchUploader()
{
    TRACE("PatchUploader::ctor");
}


PatchUploader::~PatchUploader()
{
    TRACE("PatchUploader::dtor");
}


static bool Send(HttpClient& httpClient, const void* ptr, size_t len)
{
    const char* s1 = (const char*)ptr;
    const char* s2 = s1 + len;
    while (s1 < s2)
    {
        ssize_t n = httpClient.send(s1, s2 - s1);
        if (n > 0)
        {
            s1 += n;
        }
        else if (n < 0)
        {
            Logger::instance().error("send failed.");
            return false;
        }
    }
    return true;
}


static bool Recv(HttpClient& httpClient, void* ptr, size_t len)
{
    char* r1 = (char*)ptr;
    char* r2 = r1 + len;
    while (r1 < r2)
    {
        ssize_t n = httpClient.recv(r1, r2 - r1);
        if (n > 0)
        {
            r1 += n;
        }
        else if (n < 0)
        {
            Logger::instance().error("recv failed.");
            return false;
        }
    }
    return true;
}


static bool Recv(HttpClient& httpClient, StringBuffer& buf)
{
    int state = 0;
    int pos = 0;
    while (state < 4)
    {
        char tmp[1024];
        ssize_t n = httpClient.recv(tmp, sizeof(tmp));
        if (n > 0)
        {
            int m = buf.len();
            buf.append(tmp, n);
            char* s1 = buf.ptr() + m;
            char* s2 = s1 + n;
            while (state < 4 && s1 < s2)
            {
                switch (state)
                {
                case 0:
                case 2:
                    if (*s1 == '\r')
                        state++;
                    else
                        state = 0;
                    break;
                case 1:
                case 3:
                    if (*s1 == '\n')
                    {
                        state++;
                        int len = (int)(((s1 - 1) - buf.ptr()) - pos);
                        TRACE1("PatchUploader::run: Response: %.*s", len, buf.ptr() + pos);
                        pos += len + 2;
                    }
                    else
                        state = 0;
                    break;
                default:
                    break;
                }
                s1++;
            }
        }
        else if (n < 0)
        {
            Logger::instance().error("recv failed.");
            return false;
        }
    }
    return true;
}


static bool ParseStatusLine(const char*& start, const char* end, int& status, Glib::ustring& description)
{
    const char* cr = (const char*)memchr(start, '\r', end - start);
    if (!cr)
    {
        Logger::instance().trace("PatchUploader: ParseStatusLine: CR not found.");
        return false;
    }
    if (*(cr + 1) != '\n')
    {
        Logger::instance().trace("PatchUploader: ParseStatusLine: LF not found.");
        return false;
    }
    const char* sp = (const char*)memchr(start, ' ', cr - start);
    if (!sp)
    {
        Logger::instance().trace("PatchUploader: ParseStatusLine: SP1 not found.");
        return false;
    }
    const char* stop = NULL;
    status = (int)strtoul(sp + 1, (char**)&stop, 10);
    if (stop <= sp + 1 || *stop != ' ')
    {
        Logger::instance().trace("PatchUploader: ParseStatusLine: SP2 not found.");
        return false;
    }
    description.assign(sp + 1, cr - (sp + 1));
    start = cr + 2;
    return true;
}


// cf. https://github.com/xenserver/xenadmin/CommandLib/thinCLIProtocol.cs
bool PatchUploader::run(Session& session, const char* path)
{
    TRACE("PatchUploader::run", "path=\"%s\"", path);

    bool retval = false;

    {
        _file = File::create(path, "r");

        if (!_file->open())
        {
            Logger::instance().error("%s: %s", _file->path(), strerror(_file->error()));
            goto done;
        }

        const ConnectSpec& cs = session.getConnectSpec();
        Glib::ustring url = Glib::ustring::compose(
            "https://%1/cli",
            cs.hostname);

        Glib::ustring pw = cs.descramblePassword();

        RefPtr<HttpClient> httpClient = HttpClient::create();
        httpClient->init();
        httpClient->setFreshConnect();
        httpClient->setUrl(url.c_str());
        httpClient->setVerbose(true);
        if (!httpClient->connect())
        {
            Logger::instance().error("connect failed.");
            goto done;
        }

        StringBuffer body;
        body.append("update-upload\n");
        body.appendFormat("file-name=%s\n", _file->path());
        body.appendFormat("username=%s\n", cs.username.c_str());
        body.appendFormat("password=%s", pw.c_str());

        pw.clear();

        StringBuffer obuf;
        obuf.append("POST /cli HTTP/1.0\r\n");
        obuf.appendFormat("content-length: %d\r\n", body.len());
        obuf.append("\r\n");
        obuf.append(body.str());

        if (!Send(*httpClient, obuf.str(), obuf.len()))
        {
            goto done;
        }

        static const char magic[] = { "XenSource thin CLI protocol" };

        StringBuffer ibuf;
        ibuf.resize(strlen(magic));
        if (!Recv(*httpClient, ibuf.ptr(), ibuf.size()))
        {
            goto done;
        }
        if (memcmp(ibuf.ptr(), magic, ibuf.size()))
        {
            Logger::instance().error("magic mismatch.");
            goto done;
        }
        int remoteMajor;
        int remoteMinor;
        if (!Recv(*httpClient, &remoteMajor, sizeof(int)))
        {
            goto done;
        }
        if (!Recv(*httpClient, &remoteMinor, sizeof(int)))
        {
            goto done;
        }
        int major = 0;
        int minor = 1;
        if (!Send(*httpClient, magic, strlen(magic)))
        {
            goto done;
        }
        if (!Send(*httpClient, &major, sizeof(int)))
        {
            goto done;
        }
        if (!Send(*httpClient, &minor, sizeof(int)))
        {
            goto done;
        }

        int length;
        int command1;
        int command2;
        if (!Recv(*httpClient, &length, sizeof(int)))
        {
            goto done;
        }
        if (!Recv(*httpClient, &command1, sizeof(int)))
        {
            goto done;
        }
        if (command1 != 9)
        {
            Logger::instance().error("Command(9) was expected.");
            goto done;
        }
        if (!Recv(*httpClient, &command2, sizeof(int)))
        {
            goto done;
        }
        if (command2 != 13)
        {
            Logger::instance().error("PUT(13) was expected.");
            goto done;
        }
        int length1;
        if (!Recv(*httpClient, &length1, sizeof(int)))
        {
            goto done;
        }
        ibuf.resize(length1);
        if (!Recv(*httpClient, ibuf.ptr(), ibuf.size()))
        {
            goto done;
        }
        Glib::ustring target(ibuf.ptr(), ibuf.size());
        int length2;
        if (!Recv(*httpClient, &length2, sizeof(int)))
        {
            goto done;
        }
        ibuf.resize(length2);
        if (!Recv(*httpClient, ibuf.ptr(), ibuf.size()))
        {
            goto done;
        }
        Glib::ustring path(ibuf.ptr(), ibuf.size());
        url = Glib::ustring::compose("https://%1%2", cs.hostname, path);

        RefPtr<HttpClient> httpClient2 = HttpClient::create();
        httpClient2->init();
        httpClient2->setFreshConnect();
        httpClient2->setUrl(url.c_str());
        httpClient2->setVerbose(true);
        if (!httpClient2->connect())
        {
            Logger::instance().error("connect failed.");
            goto done;
        }

        obuf.setLength(0);
        obuf.appendFormat("PUT %s HTTP/1.0\r\n", path.c_str());
        obuf.appendFormat("Content-Length: %zu\r\n", _file->size());
        obuf.append("\r\n");

        if (!Send(*httpClient2, obuf.str(), obuf.len()))
        {
            goto done;
        }

        ibuf.setLength(0);
        if (!Recv(*httpClient2, ibuf))
        {
            goto done;
        }
        const char* r1 = ibuf.str();
        const char* r9 = r1 + ibuf.len();
        fprintf(stderr, "%.*s", (int)ibuf.len(), ibuf.str());
        int status = -1;
        Glib::ustring desc;
        if (!ParseStatusLine(r1, r9, status, desc))
        {
            goto done;
        }
        TRACEPUT("status=%d", status);

        obuf.resize(65536);
        while (1)
        {
            size_t n = _file->read(obuf.ptr(), obuf.size());
            if (!n)
            {
                retval = true;
                break;
            }
            if (!Send(*httpClient2, obuf.ptr(), n))
            {
                retval = false;
                break;
            }
        }

        _file->close();

        int result = retval ? 5 : 6;
        if (!Send(*httpClient, &result, sizeof(int)))
        {
            goto done;
        }
    }

done:

    _file.reset();

    TRACEPUT("return=%s", retval ? "true" : "false");

    return retval;
}


bool PatchUploader::onSuccess(HttpClient&, int status)
{
    TRACE("PatchUploader::onSuccess", "status=%d", status);
    return status == 200 ? true : false;
}


bool PatchUploader::onFailure(HttpClient&, const char* error)
{
    Logger::instance().error("%s", error);
    return false;
}


bool PatchUploader::onCancelled(HttpClient&)
{
    Logger::instance().warn("Patch upload cancelled.");
    return false;
}


bool PatchUploader::write(HttpClient&, const void* ptr, size_t len)
{
    const char* s1 = (const char*)ptr;
    const char* s9 = s1 + len;
    while (s1 < s9)
    {
        const char* s3;
        const char* s2 = (const char*)memchr(s1, '\r', s9 - s1);
        if (s2)
        {
            s3 = s2 + 1;
            if (s3 < s9 && *s3 == '\n')
            {
                s3++;
            }
        }
        else
        {
            s2 = (const char*)memchr(s1, '\n', s9 - s1);
            if (s2)
            {
                s3 = s2 + 1;
            }
            else
            {
                s3 = s2 = s9;
            }
        }
        Logger::instance().trace("[%.*s]", (int)(s2 - s1), s1);
        s1 = s3;
    }
    return true;
}


size_t PatchUploader::read(HttpClient&, void* ptr, size_t len)
{
    return _file->read(ptr, len);
}


void PatchUploader::rewind(HttpClient&)
{
    _file->rewind();
}
