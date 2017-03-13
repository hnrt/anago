// Copyright (C) 2017 Hideaki Narita


#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "JsonLexer.h"


using namespace hnrt;


JsonLexer::JsonLexer(FILE* fp)
    : _fp(fp)
    , _line(1)
    , _b(0)
    , _c(0)
    , _sym(0)
{
    memset(_buf, 0, sizeof(_buf));
    read();
}


void JsonLexer::next()
{
    if (_c == EOF)
    {
        _sym = EOF;
        return;
    }
    while (isWhitespace())
    {
        read();
        if (_c == EOF)
        {
            _sym = EOF;
            return;
        }
    }

    if (_c == '\"')
    {
        _sym = Json::STRING;
        read();
        char* p1 = _buf;
        char* p2 = _buf + sizeof(_buf) - 1;
        while (_c != '\"')
        {
            if (_c == EOF)
            {
                throw Glib::ustring::compose("Line %1: Unexpected EOF.", _line);
            }
            else if (p1 == p2)
            {
                throw Glib::ustring::compose("Line %1: Too long string.", _line);
            }
            *p1++ = _c;
            read();
        }
        *p1 = '\0';
        read();
    }
    else if (isdigit(_c))
    {
        _sym = Json::NUMBER;
        char* p1 = _buf;
        char* p2 = _buf + 32;
        *p1++ = _c;
        read();
        while (_c != EOF && isdigit(_c))
        {
            if (p1 == p2)
            {
                throw Glib::ustring::compose("Line %1: Too long number.", _line);
            }
            *p1++ = _c;
            read();
        }
        *p1 = '\0';
    }
    else if (isalpha(_c))
    {
        char* p1 = _buf;
        char* p2 = _buf + 32;
        *p1++ = _c;
        read();
        while (_c != EOF && isalpha(_c))
        {
            if (p1 == p2)
            {
                throw Glib::ustring::compose("Line %1: Too long token.", _line);
            }
            *p1++ = _c;
            read();
        }
        *p1 = '\0';
        if (!strcmp(_buf, "false"))
        {
            _sym = Json::VALUE_FALSE;
        }
        else if (!strcmp(_buf, "null"))
        {
            _sym = Json::VALUE_NULL;
        }
        else if (!strcmp(_buf, "true"))
        {
            _sym = Json::VALUE_TRUE;
        }
        else
        {
            throw Glib::ustring::compose("Line %1: Unknown token: %2", _line, _buf);
        }
    }
    else if (_c == ':'
             || _c == ','
             || _c == '{'
             || _c == '}'
             || _c == '['
             || _c == ']')
    {
        _sym = _c;
        read();
    }
    else
    {
        _buf[0] = _c;
        _buf[1] = '\0';
        throw Glib::ustring::compose("Line %1: Invalid char: %2", _line, _buf);
    }
}


bool JsonLexer::isWhitespace() const
{
    return _c == 0x20 || _c == 0x09 || _c == 0x0a || _c == 0x0d;
}


void JsonLexer::read()
{
    if (_b == '\n')
    {
        _line++;
    }
    _b = _c;
    _c = getc(_fp);
}
