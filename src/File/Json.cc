// Copyright (C) 2017 Hideaki Narita


#include <stdio.h>
#include "JsonParser.h"
#include "JsonWriter.h"


using namespace hnrt;


Json::Json()
{
}


Json::~Json()
{
}


void Json::load(FILE* fp)
{
    JsonLexer lexer(fp);
    JsonParser parser(lexer, *this);
    parser.run();
}


void Json::save(FILE* fp)
{
    JsonWriter writer(fp, *this);
    writer.write();
}


//////////////////////////////////////////////////////////////////////
//
// O B J E C T
//
//////////////////////////////////////////////////////////////////////


Json::Object::Object()
{
}


Json::Object::Object(const MemberArray& members)
{
    for (MemberArray::const_iterator iter = members.begin(); iter != members.end(); iter++)
    {
        _members.push_back(*iter);
    }
}


Json::Object::~Object()
{
}


void Json::Object::add(const char* key)
{
    _members.push_back(RefPtr<Member>(new Member(Glib::ustring(key), RefPtr<Value>(new Value()))));
}


void Json::Object::add(const char* key, const char* value)
{
    _members.push_back(RefPtr<Member>(new Member(Glib::ustring(key), RefPtr<Value>(new Value(value)))));
}


void Json::Object::add(const char* key, long value)
{
    _members.push_back(RefPtr<Member>(new Member(Glib::ustring(key), RefPtr<Value>(new Value(value)))));
}


void Json::Object::add(const char* key, double value)
{
    _members.push_back(RefPtr<Member>(new Member(Glib::ustring(key), RefPtr<Value>(new Value(value)))));
}


void Json::Object::add(const char* key, bool value)
{
    _members.push_back(RefPtr<Member>(new Member(Glib::ustring(key), RefPtr<Value>(new Value(value)))));
}


void Json::Object::add(const char* key, const RefPtr<Object>& value)
{
    _members.push_back(RefPtr<Member>(new Member(Glib::ustring(key), RefPtr<Value>(new Value(value)))));
}


void Json::Object::add(const char* key, const Array& value)
{
    _members.push_back(RefPtr<Member>(new Member(Glib::ustring(key), RefPtr<Value>(new Value(value)))));
}


const RefPtr<Json::Value> Json::Object::get(const char* key) const
{
    for (MemberArray::size_type index = 0; index < _members.size(); index++)
    {
        if (_members[index]->key() == key)
        {
            return _members[index]->value();
        }
    }
    return RefPtr<Value>();
}


//////////////////////////////////////////////////////////////////////
//
// V A L U E
//
//////////////////////////////////////////////////////////////////////


Json::Value::Value()
    : _type(NULLVALUE)
{
}


Json::Value::Value(Type type, const char* value)
    : _type(type)
    , _string(value ? value : "")
{
}


Json::Value::Value(const char* value)
    : _type(STRING)
    , _string(value ? value : "")
{
}


Json::Value::Value(long value)
    : _type(NUMBER)
    , _string(Glib::ustring::compose("%1", value))
{
}


Json::Value::Value(double value)
    : _type(NUMBER)
    , _string(Glib::ustring::compose("%1", value))
{
}


Json::Value::Value(bool value)
    : _type(BOOLEAN)
    , _string(value ? "true" : "false")
{
}


Json::Value::Value(const RefPtr<Object>& value)
    : _type(OBJECT)
    , _object(value)
{
}


Json::Value::Value(const Array& value)
    : _type(ARRAY)
{
    for (Array::const_iterator iter = value.begin(); iter != value.end(); iter++)
    {
        _array.push_back(*iter);
    }
}


Json::Value::~Value()
{
}


bool Json::Value::isInteger() const
{
    return _type == NUMBER && _string.find('.') <= 0;
}


bool Json::Value::isFloatingPoint() const
{
    return _type == NUMBER && _string.find('.') > 0;
}


long Json::Value::integer() const
{
    return strtol(_string.c_str(), NULL, 10);
}


double Json::Value::floatingPoint() const
{
    return strtod(_string.c_str(), NULL);
}


bool Json::Value::boolean() const
{
    return _string == "true" ? true : false;
}


//////////////////////////////////////////////////////////////////////
//
// M E M B E R
//
//////////////////////////////////////////////////////////////////////


Json::Member::Member(const Glib::ustring& key, const RefPtr<Value>& value)
    : _key(key)
    , _value(value)
{
}


Json::Member::~Member()
{
}
