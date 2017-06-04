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
        virtual bool isActive() const { return _curl ? true : false; }
        virtual void setHttpVersion(const char*);
        virtual void setUrl(const char*);
        virtual void setMethod(Method);
        virtual void setCredentials(const char*, const char*);
        virtual void setPost(const void*, size_t);
        virtual void followLocation();
        virtual void setUpload(size_t);
        virtual void removeExpectHeader();
        virtual void setTcpNoDelay(bool = true);
        virtual void setVerbose(bool = true);
        virtual bool run(HttpClientHandler&);
        virtual void cancel();
        virtual int getStatus() const { return _status; }
        virtual int getResult() const { return _result; }
        virtual const char* getError() const { return _errbuf; }
        virtual double getContentLength() const { return _contentLength; }
        virtual bool connect();
        virtual int getSocket() const;
        virtual ssize_t recv(void*, size_t);
        virtual ssize_t send(const void*, size_t);

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
        CURLcode _result;
        int _status;
        double _contentLength;
        char _errbuf[CURL_ERROR_SIZE];
    };
}


#endif //!HNRT_HTTPCLIENTIMPL_H
