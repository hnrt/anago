// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_HTTPCLIENTIMPL_H
#define HNRT_HTTPCLIENTIMPL_H


#include <curl/curl.h>
#include "Base/StringBuffer.h"
#include "Base/Time.h"
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
        virtual void setTimeout(long);
        virtual void setMaxConnects(int);
        virtual void setFreshConnect(bool = true);
        virtual void setForbidReuse(bool = true);
        virtual void setHttpVersion(const char*);
        virtual void setUrl(const char*);
        virtual void setMethod(Method);
        virtual void setCredentials(const char*, const char*);
        virtual void setPost(const void*, size_t);
        virtual void followLocation();
        virtual void setUpload(size_t);
        virtual void removeHeader(const char*);
        virtual void removeExpectHeader();
        virtual void setTcpNoDelay(bool = true);
        virtual void setVerbose(bool = true);
        virtual void setReadFunction(const ReadFunction&);
        virtual void setRewindFunction(const RewindFunction&);
        virtual void setWriteFunction(const WriteFunction&);
        virtual void resetReadFunction();
        virtual void resetRewindFunction();
        virtual void resetWriteFunction();
        virtual bool run();
        virtual void cancel();
        virtual long remainingTime() const;
        virtual bool timedOut() const;
        virtual int getStatus() const { return _status; }
        virtual int getResult() const { return _result; }
        virtual const char* getError() const { return _errbuf; }
        virtual double getContentLength() const { return _contentLength; }
        virtual bool connect();
        virtual int getSocket();
        virtual int canRecv(long);
        virtual int canSend(long);
        virtual ssize_t recv(void*, size_t);
        virtual ssize_t send(const void*, size_t);
        virtual bool recvResponse();
        virtual const HeaderMap& getResponseHeaderMap() const;
        virtual int getResponseBodyLength() const;
        virtual int getResponseBody(void*, size_t) const;

        struct Response
        {
            struct Version
            {
                int major;
                int minor;
            };

            Version version;
            int& statusCode;
            StringBuffer reason;
            HttpClient::HeaderMap& headerMap;
            StringBuffer& body;
            StringBuffer key;
            StringBuffer val;

            inline Response(HttpClientImpl&);

        private:

            Response(const Response&);
            void operator =(const Response&);
        };

    protected:

        HttpClientImpl(const HttpClientImpl&);
        void operator =(const HttpClientImpl&);
        static curlioerr ioControl(CURL*, curliocmd, HttpClientImpl*);
        static size_t receiveData(void*, size_t, size_t, HttpClientImpl*);
        static size_t sendData(void*, size_t, size_t, HttpClientImpl*);

        CURL* _curl;
        struct curl_slist *_headers;
        ReadFunction _readFunction;
        RewindFunction _rewindFunction;
        WriteFunction _writeFunction;
        long _timeout;
        Time _expiry;
        volatile bool _cancelled;
        CURLcode _result;
        int _socket;
        int _status;
        double _contentLength;
        char _errbuf[CURL_ERROR_SIZE];
        HeaderMap _responseHeaderMap;
        StringBuffer _responseBody;
    };

    inline HttpClientImpl::Response::Response(HttpClientImpl& client)
        : statusCode(client._status)
        , headerMap(client._responseHeaderMap)
        , body(client._responseBody)
    {
        version.major = 0;
        version.minor = 0;
    }
}


#endif //!HNRT_HTTPCLIENTIMPL_H
