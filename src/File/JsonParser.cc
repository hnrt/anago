// Copyright (C) 2017 Hideaki Narita


#include "JsonParser.h"


using namespace hnrt;


JsonParser::JsonParser(JsonLexer& lex)
    : _lex(lex)
{
    _map.insert(ParseValueMap::value_type(Json::NULLVALUE, &JsonParser::parseImmediateValue));
    _map.insert(ParseValueMap::value_type(Json::BOOLEAN, &JsonParser::parseImmediateValue));
    _map.insert(ParseValueMap::value_type(Json::STRING, &JsonParser::parseImmediateValue));
    _map.insert(ParseValueMap::value_type(Json::NUMBER, &JsonParser::parseImmediateValue));
    _map.insert(ParseValueMap::value_type(Json::BEGIN_OBJECT, &JsonParser::parseObject));
    _map.insert(ParseValueMap::value_type(Json::BEGIN_ARRAY, &JsonParser::parseArray));
}


RefPtr<Json> JsonParser::run()
{
    RefPtr<Json> doc;

    _lex.next();

    if (parseValue(doc))
    {
        if (doc->type() == Json::OBJECT || doc->type() == Json::ARRAY)
        {
            //OK
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

    return doc;
}


bool JsonParser::parseValue(RefPtr<Json>& value)
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


bool JsonParser::parseImmediateValue(RefPtr<Json>& value)
{
    value = Json::create((Json::Type)_lex.sym(), _lex.str());
    _lex.next();
    return true;
}


bool JsonParser::parseObject(RefPtr<Json>& value)
{
    if (_lex.sym() == Json::BEGIN_OBJECT)
    {
        _lex.next();
    }
    else
    {
        return false;
    }

    value = Json::create(Json::OBJECT);

    RefPtr<Json::Member> member;

    if (parseMember(member))
    {
        value->add(member);
        while (_lex.sym() == Json::VALUE_SEPARATOR)
        {
            _lex.next();
            if (parseMember(member))
            {
                value->add(member);
            }
            else
            {
                break;
            }
        }
    }

    if (_lex.sym() == Json::END_OBJECT)
    {
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

    RefPtr<Json> value;

    if (parseValue(value))
    {
        member = Json::Member::create(key, value);
        return true;
    }
    else
    {
        throw Glib::ustring::compose("Line %1: Parse member failed: Missing value.", _lex.line());
    }
}


bool JsonParser::parseArray(RefPtr<Json>& value)
{
    if (_lex.sym() == Json::BEGIN_ARRAY)
    {
        _lex.next();
    }
    else
    {
        return false;
    }

    value = Json::create(Json::ARRAY);

    RefPtr<Json> element;

    if (parseValue(element))
    {
        value->add(element);
        while (_lex.sym() == Json::VALUE_SEPARATOR)
        {
            _lex.next();
            if (parseValue(element))
            {
                value->add(element);
            }
            else
            {
                break;
            }
        }
    }

    if (_lex.sym() == Json::END_ARRAY)
    {
        _lex.next();
        return true;
    }
    else
    {
        throw Glib::ustring::compose("Line %1: Parse array failed: Missing end-array.", _lex.line());
    }
}
