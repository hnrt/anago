// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_HTTPCLIENTIMPL_H
#define HNRT_HTTPCLIENTIMPL_H


#include <curl/curl.h>
#include "HttpClient.h"


namespace hnrt
{
    class HttpClientImpl
        : public HttpClient
    {
    public:

        HttpClientImpl();
        virtual ~HttpClientImpl();
        virtual void init();
        virtual void fini();
        virtual void setHttpVersion(const char*);
        virtual void setUrl(const char*);
        virtual void setMethod(Method);
        virtual void followLocation();
        virtual void setUpload(size_t);
        virtual void removeExpectHeader();
        virtual void setVerbose(bool = true);
        virtual bool run(HttpClientHandler&);
        virtual void cancel();
        virtual int getStatus() const { return _status; }
        virtual const char* getError() const { return _errbuf; }
        virtual double getContentLength() const { return _contentLength; }

    protected:

        HttpClientImpl(const HttpClientImpl&);
        void operator =(const HttpClientImpl&);
        static curlioerr ioControl(CURL*, curliocmd, HttpClientImpl*);
        static size_t receiveData(void*, size_t, size_t, HttpClientImpl*);
        static size_t sendData(void*, size_t, size_t, HttpClientImpl*);

        CURL* _curl;
        struct curl_slist *_chunk;
        HttpClientHandler* _handler;
        volatile bool _cancelled;
        int _status;
        double _contentLength;
        char _errbuf[CURL_ERROR_SIZE];
    };
}


#endif //!HNRT_HTTPCLIENTIMPL_H
