// Copyright (C) 2017 Hideaki Narita


#include "Base/StringBuffer.h"
#include "Logger/Logger.h"
#include "HttpClientDefaultHandler.h"


using namespace hnrt;


HttpClientDefaultHandler::HttpClientDefaultHandler()
{
    Logger::instance().trace("HttpClientDefaultHandler::ctor");
}


HttpClientDefaultHandler::~HttpClientDefaultHandler()
{
    Logger::instance().trace("HttpClientDefaultHandler::dtor");
}


bool HttpClientDefaultHandler::onSuccess(HttpClient&, int status)
{
    Logger::instance().trace("HttpClientDefaultHandler::onSuccess: status=%d", status);
    return true;
}


bool HttpClientDefaultHandler::onFailure(HttpClient&, const char* error)
{
    Logger::instance().warn("%s", error);
    return false;
}


bool HttpClientDefaultHandler::onCancelled(HttpClient&)
{
    Logger::instance().trace("HttpClientDefaultHandler::onCancelled()");
    return false;
}


size_t HttpClientDefaultHandler::read(HttpClient&, void* ptr, size_t len)
{
    Logger::instance().trace("HttpClientDefaultHandler::read(len=%zu)", len);
    return 0;
}


bool HttpClientDefaultHandler::write(HttpClient&, const void* ptr, size_t len)
{
    if (Logger::instance().getLevel() <= LogLevel::TRACE)
    {
        StringBuffer buf;
        const char* p1 = reinterpret_cast<const char*>(ptr);
        const char* p2 = p1 + len;
        while (p1 < p2)
        {
            switch (*p1)
            {
            case '\n':
                buf.append("\\n");
                break;
            case '\r':
                buf.append("\\r");
                break;
            case '\\':
                buf.append("\\\\");
                break;
            default:
                buf.append(*p1);
                break;
            }
            p1++;
        }
        Logger::instance().trace("HttpClientDefaultHandler::write(len=%zu): %s", len, buf.str());
    }
    return true;
}


void HttpClientDefaultHandler::rewind(HttpClient&)
{
    Logger::instance().trace("HttpClientDefaultHandler::rewind()");
}
