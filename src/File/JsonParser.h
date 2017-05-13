// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_JSON_PARSER_H
#define HNRT_JSON_PARSER_H


#include <map>
#include "JsonLexer.h"


namespace hnrt
{
    class JsonParser
    {
    public:

        JsonParser(JsonLexer&);
        RefPtr<Json> run();

    private:

        typedef bool (JsonParser::*ParseValue)(RefPtr<Json>&);
        typedef std::map<int, ParseValue> ParseValueMap;

        JsonParser(const JsonParser&);
        void operator =(const JsonParser&);
        bool parseValue(RefPtr<Json>&);
        bool parseImmediateValue(RefPtr<Json>&);
        bool parseString(RefPtr<Json>&);
        bool parseObject(RefPtr<Json>&);
        bool parseMember(RefPtr<Json::Member>&);
        bool parseArray(RefPtr<Json>&);

        JsonLexer& _lex;
        ParseValueMap _map;
    };
}


#endif //!HNRT_JSON_PARSER_H
