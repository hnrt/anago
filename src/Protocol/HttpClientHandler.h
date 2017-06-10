// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_HTTPCLIENTHANDLER_H
#define HNRT_HTTPCLIENTHANDLER_H


#include <stddef.h>


namespace hnrt
{
    class HttpClient;

    class HttpClientHandler
    {
    public:

        virtual bool onSuccess(HttpClient&, int) = 0;
        virtual bool onFailure(HttpClient&, const char*) = 0;
        virtual bool onCancelled(HttpClient&) = 0;
        virtual size_t read(HttpClient&, void*, size_t) = 0;
        virtual bool write(HttpClient&, const void*, size_t) = 0;
        virtual void rewind(HttpClient&) = 0;
    };
}


#endif //!HNRT_HTTPCLIENTHANDLER_H
