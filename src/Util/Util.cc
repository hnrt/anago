// Copyright (C) 2012-2017 Hideaki Narita


#include <ctype.h>
#include <string.h>
#include "Util.h"


using namespace hnrt;


StringBuffer& hnrt::FormatSize(StringBuffer& buffer, int64_t size)
{
    const int64_t kb = 1024L;
    const int64_t mb = 1024L * kb;
    const int64_t gb = 1024L * mb;
    const int64_t tb = 1024L * gb;

    if (size >= tb)
    {
        buffer.appendFormat("%ld TB", (size + tb / 2) / tb);
    }
    else if (size >= gb)
    {
        buffer.appendFormat("%ld GB", (size + gb / 2) / gb);
    }
    else if (size >= mb)
    {
        buffer.appendFormat("%ld MB", (size + mb / 2) / mb);
    }
    else if (size >= kb)
    {
        buffer.appendFormat("%ld KB", (size + kb / 2) / kb);
    }
    else
    {
        buffer.appendFormat("%ld bytes", size);
    }

    return buffer;
}


Glib::ustring hnrt::FormatSize(int64_t size)
{
    StringBuffer buffer;
    FormatSize(buffer, size);
    return Glib::ustring(buffer);
}


StringBuffer& hnrt::FormatProgress(StringBuffer& buffer, const char* status, double progress)
{
    int n = -1;
    const char* p = strrchr(status, '(');
    if (p)
    {
        const char* p1 = p++;
        while (isdigit(*p & 0xFF))
        {
            p++;
        }
        if (*p == '.' || *p == ',')
        {
            p++;
            while (isdigit(*p & 0xFF))
            {
                p++;
            }
        }
        if (*p == '%' && *(p + 1) == ')' && *(p + 2) == '\0')
        {
            n = (int)(p1 - status);
            if (n)
            {
                n--;
            }
        }
    }
    if (n > 0)
    {
        buffer.appendFormat("%.*s (%u%%)", n, status, (int)progress);
    }
    else if (n == 0 || *status == '\0')
    {
        buffer.appendFormat("(%u%%)", (int)progress);
    }
    else
    {
        buffer.appendFormat("%s (%u%%)", status, (int)progress);
    }
    return buffer;
}


bool hnrt::StartsWith(const char* s, const char* t, ssize_t n)
{
    ssize_t m = s ? strlen(s) : 0;
    if (n < 0)
    {
        n = strlen(t);
    }
    return m >= n && strncmp(s, t, n) == 0 ? true : false;
}


bool hnrt::EndsWith(const char* s, const char* t, ssize_t n)
{
    ssize_t m = s ? strlen(s) : 0;
    if (n < 0)
    {
        n = strlen(t);
    }
    return m >= n && strncmp(s + m - n, t, n) == 0 ? true : false;
}


bool hnrt::CaseStartsWith(const char* s, const char* t, ssize_t n)
{
    ssize_t m = s ? strlen(s) : 0;
    if (n < 0)
    {
        n = strlen(t);
    }
    return m >= n && strncasecmp(s, t, n) == 0 ? true : false;
}


bool hnrt::CaseEndsWith(const char* s, const char* t, ssize_t n)
{
    ssize_t m = s ? strlen(s) : 0;
    if (n < 0)
    {
        n = strlen(t);
    }
    return m >= n && strncasecmp(s + m - n, t, n) == 0 ? true : false;
}
