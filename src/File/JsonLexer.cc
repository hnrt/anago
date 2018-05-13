// Copyright (C) 2018 Hideaki Narita


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
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
    read();
}


void JsonLexer::next()
{
    _buf.clear();

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

    if (isalpha(_c))
    {
        readLiteralName();
    }
    else if (isdigit(_c))
    {
        readNumber();
    }
    else if (_c == '-')
    {
        _buf.append((char)_c);
        read();
        if (isdigit(_c))
        {
            readNumber();
        }
        else
        {
            throw Glib::ustring::compose("Line %1: Invalid char: %2", _line, _buf.str());
        }
    }
    else if (_c == '\"')
    {
        readString();
    }
    else if (isStructuralCharacter())
    {
        _sym = _c;
        _buf.append((char)_c);
        read();
    }
    else
    {
        _buf.append((char)_c);
        throw Glib::ustring::compose("Line %1: Invalid char: %2", _line, _buf.str());
    }
}


bool JsonLexer::isWhitespace() const
{
    return _c == Json::SPACE
        || _c == Json::HORIZONTAL_TAB
        || _c == Json::LINE_FEED
        || _c == Json::CARRIAGE_RETURN;
}


bool JsonLexer::isStructuralCharacter() const
{
    return _c == Json::BEGIN_OBJECT
        || _c == Json::END_OBJECT
        || _c == Json::BEGIN_ARRAY
        || _c == Json::END_ARRAY
        || _c == Json::NAME_SEPARATOR
        || _c == Json::VALUE_SEPARATOR;
}


void JsonLexer::readLiteralName()
{
    _buf.append((char)_c);
    read();
    while (isalpha(_c))
    {
        _buf.append((char)_c);
        read();
    }
    if (!strcmp(_buf.str(), "false"))
    {
        _sym = Json::BOOLEAN;
    }
    else if (!strcmp(_buf.str(), "null"))
    {
        _sym = Json::NULLVALUE;
    }
    else if (!strcmp(_buf.str(), "true"))
    {
        _sym = Json::BOOLEAN;
    }
    else
    {
        throw Glib::ustring::compose("Line %1: Bad literal name: %2", _line, _buf.str());
    }
}


void JsonLexer::readNumber()
{
    _sym = Json::NUMBER;
    if (_c == '0')
    {
        _buf.append((char)_c);
        read();
        if (isdigit(_c))
        {
            // zero followed by digit(s) is not allowed
            throw Glib::ustring::compose("Line %1: Bad number.", _line);
        }
    }
    else
    {
        _buf.append((char)_c);
        read();
        while (isdigit(_c))
        {
            _buf.append((char)_c);
            read();
        }
    }
    if (_c == '.')
    {
        // decimal fraction part
        _buf.append((char)_c);
        read();
        if (isdigit(_c))
        {
            _buf.append((char)_c);
            read();
        }
        else
        {
            throw Glib::ustring::compose("Line %1: Bad decimal fraction part.", _line);
        }
        while (isdigit(_c))
        {
            _buf.append((char)_c);
            read();
        }
    }
    if (_c == 'e' || _c == 'E')
    {
        // exponential part
        _buf.append((char)_c);
        read();
        if (_c == '-' || _c == '+')
        {
            _buf.append((char)_c);
            read();
        }
        if (isdigit(_c))
        {
            _buf.append((char)_c);
            read();
        }
        else
        {
            throw Glib::ustring::compose("Line %1: Bad exponential part.", _line);
        }
        while (isdigit(_c))
        {
            _buf.append((char)_c);
            read();
        }
    }
}


void JsonLexer::readString()
{
    _sym = Json::STRING;
    read();
    while (_c != '\"')
    {
        if (_c == EOF)
        {
            throw Glib::ustring::compose("Line %1: Unexpected EOF.", _line);
        }
        else if (_c == '\\')
        {
            readEscapeSequence();
        }
        _buf.append((char)_c);
        read();
    }
    read();
}


void JsonLexer::readEscapeSequence()
{
    read();
    if (_c == EOF)
    {
        throw Glib::ustring::compose("Line %1: Unexpected EOF.", _line);
    }
    else if (_c == '\"'
             || _c == '\\'
             || _c == '/')
    {
        // as is
    }
    else if (_c == 'b')
    {
        _c = '\b'; // backspace
    }
    else if (_c == 'f')
    {
        _c = '\f'; // form feed
    }
    else if (_c == 'n')
    {
        _c = '\n'; // line feed
    }
    else if (_c == 'r')
    {
        _c = '\r'; // carriage return
    }
    else if (_c == 't')
    {
        _c = '\t'; // tab
    }
    else if (_c == 'u')
    {
        // uXXXX
        read();
        char s[16];
        if (isxdigit(_c))
        {
            s[0] = (char)_c;
            read();
        }
        else
        {
            throw Glib::ustring::compose("Line %1: Bad escape sequence.", _line);
        }
        if (isxdigit(_c))
        {
            s[1] = (char)_c;
            read();
        }
        else
        {
            throw Glib::ustring::compose("Line %1: Bad escape sequence.", _line);
        }
        if (isxdigit(_c))
        {
            s[2] = (char)_c;
            read();
        }
        else
        {
            throw Glib::ustring::compose("Line %1: Bad escape sequence.", _line);
        }
        if (isxdigit(_c))
        {
            s[3] = (char)_c;
        }
        else
        {
            throw Glib::ustring::compose("Line %1: Bad escape sequence.", _line);
        }
        s[4] = '\0';
        wchar_t w = (wchar_t)strtol(s, NULL, 16);
        int n = wctomb(s, w);
        if (n < 1)
        {
            throw Glib::ustring::compose("Line %1: Bad escape sequence (no mapping).", _line);
        }
        else if (n > 1)
        {
            _buf.append(s, n - 1);
        }
        _c = s[n - 1];
    }
    else
    {
        throw Glib::ustring::compose("Line %1: Bad escape sequence.", _line);
    }
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
