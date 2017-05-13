// Copyright (C) 2012-2017 Hideaki Narita


#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "MsgItem.h"
#include "PoFile.h"


using namespace hnrt;


MsgItem::MsgItem()
    : _headerLine(-1)
    , _headerCount(0)
    , _locations()
    , _msgidLine(-1)
    , _msgidCount(0)
    , _msgid()
    , _msgid2Line(-1)
    , _msgid2Count(0)
    , _msgid2()
    , _msgstrLine(-1)
    , _msgstrCount(0)
    , _msgstr()
    , _msgstr2Line(-1)
    , _msgstr2Count(0)
    , _msgstr2()
{
}


MsgItem::MsgItem(const MsgItem& src)
    : _headerLine(src._headerLine)
    , _headerCount(src._headerCount)
    , _locations(src._locations.size())
    , _msgidLine(src._msgidLine)
    , _msgidCount(src._msgidCount)
    , _msgid(src._msgid)
    , _msgid2Line(src._msgid2Line)
    , _msgid2Count(src._msgid2Count)
    , _msgid2(src._msgid2)
    , _msgstrLine(src._msgstrLine)
    , _msgstrCount(src._msgstrCount)
    , _msgstr(src._msgstr)
    , _msgstr2Line(src._msgstr2Line)
    , _msgstr2Count(src._msgstr2Count)
    , _msgstr2(src._msgstr2)
{
    for (size_t i = 0; i < src._locations.size(); i++)
    {
        _locations[i] = src._locations[i];
    }
}


MsgItem::~MsgItem()
{
    clear();
}


void MsgItem::clear()
{
    _headerLine = -1;
    _headerCount = 0;
    _locations.clear();
    _msgidLine = -1;
    _msgidCount = 0;
    _msgid.clear();
    _msgid2Line = -1;
    _msgid2Count = 0;
    _msgid2.clear();
    _msgstrLine = -1;
    _msgstrCount = 0;
    _msgstr.clear();
    _msgstr2Line = -1;
    _msgstr2Count = 0;
    _msgstr2.clear();
}


void MsgItem::addLocation(const char* s)
{
    _locations.push_back(std::string(s));
}


void MsgItem::addLocation(const char* s, size_t n)
{
    _locations.push_back(std::string(s, n));
}


void MsgItem::addMsgid(const char* s, size_t n)
{
    _msgid.append(s, n);
}


void MsgItem::addMsgid2(const char* s, size_t n)
{
    _msgid2.append(s, n);
}


void MsgItem::addMsgstr(const char* s, size_t n)
{
    _msgstr.append(s, n);
}


void MsgItem::addMsgstr2(const char* s, size_t n)
{
    _msgstr2.append(s, n);
}


bool MsgItem::updateRevisionDate(PoFile& file)
{
    static const char signature[] = { "PO-Revision-Date: " };
    static const size_t sigLen = strlen(signature);

    for (int i = 0; i < _msgstrCount; i++)
    {
        int j = _msgstrLine + i;
        const char* s = file[j];
        if (s[0] == '\"' && !strncmp(&s[1], signature, sigLen))
        {
            struct tm tmbuf = { 0 };
            time_t now = time(NULL);
            localtime_r(&now, &tmbuf);
            int tzSign = timezone < 0 ? '+' : '-';
            long tzAbs = abs(timezone);
            char rd[256];
            sprintf(rd, "\"%s%04d-%02d-%02d %02d:%02d%c%02ld%02ld\\n\"\n",
                    signature,
                    tmbuf.tm_year + 1900, tmbuf.tm_mon + 1, tmbuf.tm_mday, tmbuf.tm_hour, tmbuf.tm_min,
                    tzSign, tzAbs / 3600, (tzAbs % 3600) / 60);
            file.replaceLine(j, rd);
            const char* p1 = strstr(_msgstr.c_str(), signature);
            if (p1)
            {
                const char* p2 = strstr(p1 + sigLen, "\\n");
                if (p2)
                {
                    std::string x;
                    x.append(_msgstr.c_str(), p1 - _msgstr.c_str());
                    x.append(rd + 1, strlen(rd) - 5);
                    x.append(p2);
                    _msgstr = x;
                }
            }
            //fprintf(stderr, "#t(%ld)=%s#_msgstr=(%s)\n", timezone, rd.str(), _msgstr.str());
            return true;
        }
    }
    return false;
}
