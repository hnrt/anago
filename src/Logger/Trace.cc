// Copyright (C) 2012-2017 Hideaki Narita


#include <string.h>
#include "Base/StringBuffer.h"
#include "XenServer/XenObject.h"
#include "Logger.h"
#include "Trace.h"


using namespace hnrt;


inline static int GetInsertionPosition(const char* src, const void* ptr)
{
    if (ptr)
    {
        const char* sep = strchr(src, ':');
        if (sep && *(sep + 1) == ':')
        {
            return static_cast<int>(sep - src);
        }
    }
    return -1;
}


inline static void SetName(Glib::ustring& dst, const char* src, const void* ptr)
{
    int pos = GetInsertionPosition(src, ptr);
    if (pos >= 0)
    {
        StringBuffer buf;
        buf.format("%.*s@%zx%s", pos, src, ptr, src + pos);
        dst.assign(buf);
    }
    else
    {
        dst.assign(src);
    }
}


Trace::Trace(const void* ptr, const char* fmt, ...)
    : _log(Logger::instance())
{
    StringBuffer buf;
    va_list argList;
    va_start(argList, fmt);
    buf.formatV(fmt, argList);
    va_end(argList);
    SetName(_name, buf, ptr);
    _log.trace("%s: Started.", _name.c_str());
}


Trace::~Trace()
{
    _log.trace("%s: Finished.", _name.c_str());
}


void Trace::put(const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    _log.trace2(_name.c_str(), format, argList);
    va_end(argList);
}


XenObjectText::XenObjectText(const XenObject& object)
{
    StringBuffer buf;
    buf.format("%s@%zx", GetXenObjectTypeText(object), &object);
    ptr = new char[buf.len() + 1];
    strcpy(ptr, buf);
}


XenObjectText::~XenObjectText()
{
    delete[] ptr;
}
