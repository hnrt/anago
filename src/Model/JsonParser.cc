// Copyright (C) 2017 Hideaki Narita


#include "JsonParser.h"


using namespace hnrt;


JsonParser::JsonParser(JsonLexer& lex, Json& doc)
    : _lex(lex)
    , _doc(doc)
{
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


bool JsonParser::parseObject(RefPtr<Json::Object>& object)
{
    if (_lex.sym() == '{')
    {
        _lex.next();
    }
    else
    {
        return false;
    }

    object = RefPtr<Json::Object>(new Json::Object());

    RefPtr<Json::Member> member;
    if (parseMember(member))
    {
        object->members().push_back(member);
        while (_lex.sym() == ',')
        {
            _lex.next();
            if (parseMember(member))
            {
                object->members().push_back(member);
            }
            else
            {
                break;
            }
        }
    }

    if (_lex.sym() == '}')
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

    if (_lex.sym() == ':')
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


bool JsonParser::parseValue(RefPtr<Json::Value>& value)
{
    if (_lex.sym() == Json::VALUE_FALSE
        || _lex.sym() == Json::VALUE_NULL
        || _lex.sym() == Json::VALUE_TRUE)
    {
        value = RefPtr<Json::Value>(new Json::Value());
        value->set((Json::Type)_lex.sym());
        _lex.next();
        return true;
    }
    else if (_lex.sym() == Json::NUMBER)
    {
        value = RefPtr<Json::Value>(new Json::Value());
        value->set(strtol(_lex.str(), NULL, 10));
        _lex.next();
        return true;
    }
    else if (_lex.sym() == Json::STRING)
    {
        value = RefPtr<Json::Value>(new Json::Value());
        value->set(Glib::ustring(_lex.str()));
        _lex.next();
        return true;
    }
    RefPtr<Json::Object> object;
    if (parseObject(object))
    {
        value = RefPtr<Json::Value>(new Json::Value());
        value->set(object);
        return true;
    }
    value = RefPtr<Json::Value>(new Json::Value());
    if (parseArray(value->array()))
    {
        return true;
    }
    return false;
}


bool JsonParser::parseArray(Json::Array& array)
{
    if (_lex.sym() == '[')
    {
        _lex.next();
    }
    else
    {
        return false;
    }

    RefPtr<Json::Value> value;

    if (parseValue(value))
    {
        array.push_back(value);
        while (_lex.sym() == ',')
        {
            _lex.next();
            if (parseValue(value))
            {
                array.push_back(value);
            }
            else
            {
                break;
            }
        }
    }

    if (_lex.sym() == ']')
    {
        _lex.next();
        return true;
    }
    else
    {
        throw Glib::ustring::compose("Line %1: Parse array failed: Missing end-array.", _lex.line());
    }
}
