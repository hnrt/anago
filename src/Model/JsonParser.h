// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_JSON_PARSER_H
#define HNRT_JSON_PARSER_H


#include "JsonLexer.h"


namespace hnrt
{
    class JsonParser
    {
    public:

        JsonParser(JsonLexer&, Json&);
        void run();

    private:

        JsonParser(const JsonParser&);
        void operator =(const JsonParser&);
        bool parseObject(RefPtr<Json::Object>&);
        bool parseMember(RefPtr<Json::Member>&);
        bool parseValue(RefPtr<Json::Value>&);
        bool parseArray(Json::Array&);
        int getNext();
        bool isWhitespace() const;
        int getChar();

        JsonLexer& _lex;
        Json& _doc;
    };
}


#endif //!HNRT_JSON_PARSER_H
