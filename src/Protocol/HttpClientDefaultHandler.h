// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_HTTPCLIENTDEFAULTHANDLER_H
#define HNRT_HTTPCLIENTDEFAULTHANDLER_H


#include "HttpClientHandler.h"


namespace hnrt
{
    class HttpClientDefaultHandler
        : public HttpClientHandler
    {
    public:

        HttpClientDefaultHandler();
        virtual ~HttpClientDefaultHandler();
        virtual bool onSuccess(HttpClient&, int);
        virtual bool onFailure(HttpClient&, const char*);
        virtual bool onCancelled(HttpClient&);
        virtual size_t read(HttpClient&, void*, size_t);
        virtual bool write(HttpClient&, const void*, size_t);
        virtual void rewind(HttpClient&);

    private:

        HttpClientDefaultHandler(const HttpClientDefaultHandler&);
        void operator =(const HttpClientDefaultHandler&);
    };
}


#endif //!HNRT_HTTPCLIENTDEFAULTHANDLER_H
