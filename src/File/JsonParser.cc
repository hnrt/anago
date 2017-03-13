// Copyright (C) 2017 Hideaki Narita


#include "JsonParser.h"


using namespace hnrt;


JsonParser::JsonParser(JsonLexer& lex, Json& doc)
    : _lex(lex)
    , _doc(doc)
{
    _map.insert(ParseValueMap::value_type(Json::NULLVALUE, &JsonParser::parseImmediateValue));
    _map.insert(ParseValueMap::value_type(Json::BOOLEAN, &JsonParser::parseImmediateValue));
    _map.insert(ParseValueMap::value_type(Json::STRING, &JsonParser::parseImmediateValue));
    _map.insert(ParseValueMap::value_type(Json::NUMBER, &JsonParser::parseImmediateValue));
    _map.insert(ParseValueMap::value_type(Json::BEGIN_OBJECT, &JsonParser::parseObject));
    _map.insert(ParseValueMap::value_type(Json::BEGIN_ARRAY, &JsonParser::parseArray));
}


void JsonParser::run()
{
    _lex.next();

    RefPtr<Json::Value> value;

    if (parseValue(value))
    {
        if (value->type() == Json::OBJECT || value->type() == Json::ARRAY)
        {
            _doc.set(value);
        }
        else
        {
            throw Glib::ustring::compose("Line %1: Value must be either object or array.", _lex.line());
        }
    }
    else
    {
        throw Glib::ustring::compose("Line %1: Parse failed.", _lex.line());
    }

    if (_lex.sym() != EOF)
    {
        throw Glib::ustring::compose("Line %1: Parse failed.", _lex.line());
    }
}


bool JsonParser::parseValue(RefPtr<Json::Value>& value)
{
    ParseValueMap::const_iterator iter = _map.find(_lex.sym());
    if (iter != _map.end())
    {
        ParseValue func = iter->second;
        return (this->*func)(value);
    }
    else
    {
        return false;
    }
}


bool JsonParser::parseImmediateValue(RefPtr<Json::Value>& value)
{
    value = RefPtr<Json::Value>(new Json::Value((Json::Type)_lex.sym(), _lex.str()));
    _lex.next();
    return true;
}


bool JsonParser::parseObject(RefPtr<Json::Value>& value)
{
    if (_lex.sym() == Json::BEGIN_OBJECT)
    {
        _lex.next();
    }
    else
    {
        return false;
    }

    Json::MemberArray members;

    RefPtr<Json::Member> member;

    if (parseMember(member))
    {
        members.push_back(member);
        while (_lex.sym() == Json::VALUE_SEPARATOR)
        {
            _lex.next();
            if (parseMember(member))
            {
                members.push_back(member);
            }
            else
            {
                break;
            }
        }
    }

    if (_lex.sym() == Json::END_OBJECT)
    {
        value = RefPtr<Json::Value>(new Json::Value(RefPtr<Json::Object>(new Json::Object(members))));
        _lex.next();
        return true;
    }
    else
    {
        throw Glib::ustring::compose("Line %1: Parse object failed: Missing end-object.", _lex.line());
    }
}


bool JsonParser::parseMember(RefPtr<Json::Member>& member)
{
    Glib::ustring key;

    if (_lex.sym() == Json::STRING)
    {
        key.assign(_lex.str());
        _lex.next();
    }
    else
    {
        return false;
    }

    if (_lex.sym() == Json::NAME_SEPARATOR)
    {
        _lex.next();
    }
    else
    {
        throw Glib::ustring::compose("Line %1: Parse member failed: Missing name-separator.", _lex.line());
    }

    RefPtr<Json::Value> value;

    if (parseValue(value))
    {
        member = RefPtr<Json::Member>(new Json::Member(key, value));
        return true;
    }
    else
    {
        throw Glib::ustring::compose("Line %1: Parse member failed: Missing value.", _lex.line());
    }
}


bool JsonParser::parseArray(RefPtr<Json::Value>& value)
{
    if (_lex.sym() == Json::BEGIN_ARRAY)
    {
        _lex.next();
    }
    else
    {
        return false;
    }

    Json::Array elements;

    RefPtr<Json::Value> element;

    if (parseValue(element))
    {
        elements.push_back(element);
        while (_lex.sym() == Json::VALUE_SEPARATOR)
        {
            _lex.next();
            if (parseValue(element))
            {
                elements.push_back(element);
            }
            else
            {
                break;
            }
        }
    }

    if (_lex.sym() == Json::END_ARRAY)
    {
        value = RefPtr<Json::Value>(new Json::Value(elements));
        _lex.next();
        return true;
    }
    else
    {
        throw Glib::ustring::compose("Line %1: Parse array failed: Missing end-array.", _lex.line());
    }
}
