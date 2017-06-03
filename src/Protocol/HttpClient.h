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
        virtual void setUrl(const char*) = 0;
        virtual void setMethod(Method) = 0;
        virtual void followLocation() = 0;
        virtual void setUpload(size_t) = 0;
        virtual void removeExpectHeader() = 0;
        virtual bool run(HttpClientHandler&) = 0;
        virtual void cancel() = 0;
        virtual int getStatus() const = 0;
        virtual const char* getError() const = 0;
        virtual double getContentLength() const = 0;

    protected:

        HttpClient();
        HttpClient(const HttpClient&);
        void operator =(const HttpClient&);
    };
}


#endif //!HNRT_HTTPCLIENT_H
