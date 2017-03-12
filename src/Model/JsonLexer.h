// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_JSON_LEXER_H
#define HNRT_JSON_LEXER_H


#include <stdio.h>
#include "Json.h"


namespace hnrt
{
    class JsonLexer
    {
    public:

        JsonLexer(FILE*);
        void next();
        int sym() const { return _sym; }
        int line() const { return _line; }
        const char* str() { return _buf; }

    private:

        JsonLexer(const JsonLexer&);
        void operator =(const JsonLexer&);
        bool isWhitespace() const;
        void read();

        FILE* _fp;
        int _line;
        int _b;
        int _c;
        int _sym;
        char _buf[1024];
    };
}


#endif //!HNRT_JSON_LEXER_H
