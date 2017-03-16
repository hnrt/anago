// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_UTIL_H
#define HNRT_UTIL_H


#include <stdint.h>
#include <glibmm.h>
#include "StringBuffer.h"


namespace hnrt
{
    void FormatSize(StringBuffer& buffer, int64_t size);
    Glib::ustring FormatSize(int64_t size);
    void FormatProgress(StringBuffer& buffer, const char* status, double progress);

    bool StartsWith(const char*, const char*, ssize_t = -1);
    bool EndsWith(const char*, const char*, ssize_t = -1);
    bool CaseStartsWith(const char*, const char*, ssize_t = -1);
    bool CaseEndsWith(const char*, const char*, ssize_t = -1);
}


#endif //!HNRT_UTIL_H
