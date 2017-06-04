// Copyright (C) 2017 Hideaki Narita


#include "Base/StringBuffer.h"
#include "Logger/Logger.h"
#include "HttpClientHandler.h"


using namespace hnrt;


bool HttpClientHandler::onSuccess(HttpClient&, int status)
{
    Logger::instance().trace("HttpClientHandler::onSuccess: status=%d", status);
    return true;
}


bool HttpClientHandler::onFailure(HttpClient&, const char* error)
{
    Logger::instance().warn("%s", error);
    return false;
}


bool HttpClientHandler::onCancelled(HttpClient&)
{
    Logger::instance().trace("HttpClientHandler::onCancelled()");
    return false;
}


size_t HttpClientHandler::read(HttpClient&, void* ptr, size_t len)
{
    Logger::instance().trace("HttpClientHandler::read(len=%zu)", len);
    return 0;
}


bool HttpClientHandler::write(HttpClient&, void* ptr, size_t len)
{
    if (Logger::instance().getLevel() <= LogLevel::TRACE)
    {
        StringBuffer buf;
        char* p1 = reinterpret_cast<char*>(ptr);
        char* p2 = p1 + len;
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
        Logger::instance().trace("HttpClientHandler::write(len=%zu): %s", len, buf.str());
    }
    return true;
}


void HttpClientHandler::rewind(HttpClient&)
{
    Logger::instance().trace("HttpClientHandler::rewind()");
}
