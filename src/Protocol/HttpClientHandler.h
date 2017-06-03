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

        virtual bool onSuccess(HttpClient&, int) { return true; }
        virtual bool onFailure(HttpClient&, const char*) { return false; }
        virtual bool onCancelled(HttpClient&) { return false; }
        virtual size_t read(HttpClient&, void* ptr, size_t len) { return 0; }
        virtual bool write(HttpClient&, void* ptr, size_t len) { return true; }
        virtual void rewind(HttpClient&) {}
    };
}


#endif //!HNRT_HTTPCLIENTHANDLER_H
