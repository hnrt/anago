// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_HTTPCLIENT_H
#define HNRT_HTTPCLIENT_H


#include <stddef.h>
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

        static RefPtr<HttpClient> create();

        virtual ~HttpClient();
        virtual void init() = 0;
        virtual void fini() = 0;
        virtual bool isActive() const = 0;
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
        virtual int getStatus() const = 0;
        virtual int getResult() const = 0;
        virtual const char* getError() const = 0;
        virtual double getContentLength() const = 0;
        virtual bool connect() = 0;
        virtual int getSocket() const = 0;
        virtual ssize_t recv(void*, size_t) = 0;
        virtual ssize_t send(const void*, size_t) = 0;

    protected:

        HttpClient();
        HttpClient(const HttpClient&);
        void operator =(const HttpClient&);
    };
}


#endif //!HNRT_HTTPCLIENT_H
