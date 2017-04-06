// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONSOLECONNECTOR_H
#define HNRT_CONSOLECONNECTOR_H


#include <glibmm/thread.h>
#include <glibmm/ustring.h>
#include <curl/curl.h>
#include <map>
#include <list>
#include "Base/ByteBuffer.h"


namespace hnrt
{
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
        ssize_t send();
        size_t recv();
        Glib::ustring getRequest(const char* location, const char* authorization);
        void parseLocation(const char* location, Glib::ustring& host, Glib::ustring& absPathAndQuery);
        bool getHeaderLength(size_t&);
        bool parseHeader(size_t);
        bool extendInputBuffer(size_t sizeHint = 0);
        bool extendOutputBuffer(size_t sizeHint = 0);

        CURL* _curl;
        int _sockHost;
        ByteBuffer _ibuf;
        ByteBuffer _obuf;
        int _statusCode;
        HeaderMap _headerMap;
    };
}


#endif //!HNRT_CONSOLECONNECTOR_H
