// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_CONSOLECONNECTOR_H
#define HNRT_CONSOLECONNECTOR_H


#include <glibmm/thread.h>
#include <glibmm/ustring.h>
#include <map>
#include <list>
#include "Base/ByteBuffer.h"
#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class HttpClient;

    class ConsoleConnector
    {
    public:

        virtual ~ConsoleConnector();

    protected:

        typedef std::map<Glib::ustring, Glib::ustring> HeaderMap;
        typedef std::pair<Glib::ustring, Glib::ustring> HeaderEntry;

        ConsoleConnector();
        ConsoleConnector(const ConsoleConnector &);
        void operator =(const ConsoleConnector &);
        void open(const char* location, const char* authorization);
        void close();
        ssize_t recv();
        ssize_t send();
        bool canRecv(long = 1000L);
        bool canSend(long = 1000L);
        Glib::ustring getRequest(const char* location, const char* authorization);
        void parseLocation(const char* location, Glib::ustring& host, Glib::ustring& absPathAndQuery);
        bool getHeaderLength(size_t&);
        bool parseHeader(size_t);
        bool extendInputBuffer(size_t sizeHint = 0);
        bool extendOutputBuffer(size_t sizeHint = 0);

        RefPtr<HttpClient> _httpClient;
        ByteBuffer _ibuf;
        ByteBuffer _obuf;
        int _statusCode;
        HeaderMap _headerMap;
    };
}


#endif //!HNRT_CONSOLECONNECTOR_H
