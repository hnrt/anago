// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_HTTPCLIENT_H
#define HNRT_HTTPCLIENT_H


#include <stddef.h>
#include <glibmm/ustring.h>
#include <map>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"
#include "HttpClientHandler.h"


namespace hnrt
{
    class HttpClient
        : public RefObj
    {
    public:

        enum Method
        {
            GET,
            PUT,
            POST,
        };

        struct Header
        {
            Glib::ustring key;
            Glib::ustring value;
        };

        typedef std::map<Glib::ustring, Header> HeaderMap;
        typedef std::pair<Glib::ustring, Header> HeaderMapEntry;

        static RefPtr<HttpClient> create();

        virtual ~HttpClient();
        virtual void init() = 0;
        virtual void fini() = 0;
        virtual bool isActive() const = 0;
        virtual void setTimeout(long) = 0;
        virtual void setMaxConnects(int) = 0;
        virtual void setFreshConnect(bool = true) = 0;
        virtual void setForbidReuse(bool = true) = 0;
        virtual void setHttpVersion(const char*) = 0;
        virtual void setUrl(const char*) = 0;
        virtual void setMethod(Method) = 0;
        virtual void setCredentials(const char*, const char*) = 0;
        virtual void setPost(const void*, size_t) = 0;
        virtual void followLocation() = 0;
        virtual void setUpload(size_t) = 0;
        virtual void removeHeader(const char*) = 0;
        virtual void removeExpectHeader() = 0;
        virtual void setTcpNoDelay(bool = true) = 0;
        virtual void setVerbose(bool = true) = 0;
        virtual bool run(HttpClientHandler&) = 0;
        virtual void cancel() = 0;
        virtual long remainingTime() const = 0;
        virtual bool timedOut() const = 0;
        virtual int getStatus() const = 0;
        virtual int getResult() const = 0;
        virtual const char* getError() const = 0;
        virtual double getContentLength() const = 0;
        virtual bool connect() = 0;
        virtual int getSocket() = 0;
        virtual int canRecv(long) = 0;
        virtual int canSend(long) = 0;
        virtual ssize_t recv(void*, size_t) = 0;
        virtual ssize_t send(const void*, size_t) = 0;
        virtual bool recvResponse() = 0;
        virtual const HeaderMap& getResponseHeaderMap() const = 0;
        virtual int getResponseBodyLength() const = 0;
        virtual int getResponseBody(void*, size_t) const = 0;

    protected:

        HttpClient();
        HttpClient(const HttpClient&);
        void operator =(const HttpClient&);
    };
}


#endif //!HNRT_HTTPCLIENT_H
