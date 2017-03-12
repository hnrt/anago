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
        bool parseValue(RefPtr<Json::Value>&);
        bool parseConstant(RefPtr<Json::Value>&);
        bool parseString(RefPtr<Json::Value>&);
        bool parseNumber(RefPtr<Json::Value>&);
        bool parseObject(RefPtr<Json::Value>&);
        bool parseMember(RefPtr<Json::Member>&);
        bool parseArray(RefPtr<Json::Value>&);
        int getNext();
        bool isWhitespace() const;
        int getChar();

        typedef bool (JsonParser::*ParseValue)(RefPtr<Json::Value>&);
        typedef std::map<int, ParseValue> ParseValueMap;

        JsonLexer& _lex;
        Json& _doc;
        ParseValueMap _map;
    };
}


#endif //!HNRT_JSON_PARSER_H
