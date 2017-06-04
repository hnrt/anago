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

        virtual bool onSuccess(HttpClient&, int);
        virtual bool onFailure(HttpClient&, const char*);
        virtual bool onCancelled(HttpClient&);
        virtual size_t read(HttpClient&, void*, size_t);
        virtual bool write(HttpClient&, void*, size_t);
        virtual void rewind(HttpClient&);
    };
}


#endif //!HNRT_HTTPCLIENTHANDLER_H
