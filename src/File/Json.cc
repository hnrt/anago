// Copyright (C) 2017 Hideaki Narita


#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include "JsonParser.h"
#include "JsonWriter.h"


using namespace hnrt;


static const RefPtr<Json::Value> s_NullValue;


static Glib::ustring GetKey(const char* path, const char** next)
{
    Glib::ustring key;
    const char* dot = strchr(path, '.');
    if (dot)
    {
        key.assign(path, dot - path);
        *next = dot + 1;
    }
    else
    {
        key.assign(path);
        *next = NULL;
    }
    return key;
}


//////////////////////////////////////////////////////////////////////
//
// J S O N
//
//////////////////////////////////////////////////////////////////////


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


bool Json::getString(const char* path, Glib::ustring& retval) const
{
    return getString(_root, path, retval);
}


bool Json::getString(const RefPtr<Value>& value, const char* path, Glib::ustring& retval) const
{
    if (value)
    {
        const char* next = NULL;
        Glib::ustring key = GetKey(path, &next);
        if (!key.empty())
        {
            if (value->type() == OBJECT)
            {
                RefPtr<Object> object = value->object();
                if (object)
                {
                    RefPtr<Value> value2 = object->get(key.c_str());
                    if (next)
                    {
                        return getString(value2, next, retval);
                    }
                    else if (value2 && value2->type() == STRING)
                    {
                        retval = value2->string();
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


bool Json::getLong(const char* path, long& retval) const
{
    return getLong(_root, path, retval);
}


bool Json::getLong(const RefPtr<Value>& value, const char* path, long& retval) const
{
    if (value)
    {
        const char* next = NULL;
        Glib::ustring key = GetKey(path, &next);
        if (!key.empty())
        {
            if (value->type() == OBJECT)
            {
                RefPtr<Object> object = value->object();
                if (object)
                {
                    RefPtr<Value> value2 = object->get(key.c_str());
                    if (next)
                    {
                        return getLong(value2, next, retval);
                    }
                    else if (value2 && value2->type() == NUMBER)
                    {
                        retval = value2->integer();
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


bool Json::getInt(const char* path, int& retval) const
{
    return getInt(_root, path, retval);
}


bool Json::getInt(const RefPtr<Value>& value, const char* path, int& retval) const
{
    if (value)
    {
        const char* next = NULL;
        Glib::ustring key = GetKey(path, &next);
        if (!key.empty())
        {
            if (value->type() == OBJECT)
            {
                RefPtr<Object> object = value->object();
                if (object)
                {
                    RefPtr<Value> value2 = object->get(key.c_str());
                    if (next)
                    {
                        return getInt(value2, next, retval);
                    }
                    else if (value2 && value2->type() == NUMBER)
                    {
                        retval = (int)value2->integer();
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


bool Json::getBoolean(const char* path, bool& retval) const
{
    return getBoolean(_root, path, retval);
}


bool Json::getBoolean(const RefPtr<Value>& value, const char* path, bool& retval) const
{
    if (value)
    {
        const char* next = NULL;
        Glib::ustring key = GetKey(path, &next);
        if (!key.empty())
        {
            if (value->type() == OBJECT)
            {
                RefPtr<Object> object = value->object();
                if (object)
                {
                    RefPtr<Value> value2 = object->get(key.c_str());
                    if (next)
                    {
                        return getBoolean(value2, next, retval);
                    }
                    else if (value2 && value2->type() == BOOLEAN)
                    {
                        retval = value2->boolean();
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


bool Json::getArray(const char* path, const sigc::slot2<void, const Json&, const RefPtr<Value>&>& slot) const
{
    return getArray(_root, path, slot);
}


bool Json::getArray(const RefPtr<Value>& value, const char* path, const sigc::slot2<void, const Json&, const RefPtr<Value>&>& slot) const
{
    if (value)
    {
        const char* next = NULL;
        Glib::ustring key = GetKey(path, &next);
        if (!key.empty())
        {
            if (value->type() == OBJECT)
            {
                RefPtr<Object> object = value->object();
                if (object)
                {
                    RefPtr<Value> value2 = object->get(key.c_str());
                    if (next)
                    {
                        return getArray(value2, next, slot);
                    }
                    else if (value2 && value2->type() == ARRAY)
                    {
                        const Json::Array& array = value2->array();
                        for (Json::Array::size_type index = 0; index < array.size(); index++)
                        {
                            RefPtr<Value> value3 = array[index];
                            slot(*this, value3);
                        }
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


void Json::setString(const char* path, const Glib::ustring& setval)
{
    setString(_root, path, setval);
}


void Json::setString(RefPtr<Value>& value, const char* path, const Glib::ustring& setval)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        throw std::runtime_error("Json::setString: Key is empty.");
    }
    if (!value)
    {
        value = RefPtr<Value>(new Value(RefPtr<Object>(new Object)));
    }
    if (value->type() != OBJECT)
    {
        throw std::runtime_error("Json::setString: Value is not an object.");
    }
    if (next)
    {
        setString(value->object()->get(key.c_str()), next, setval);
    }
    else
    {
        value->object()->get(key.c_str()) = RefPtr<Value>(new Value(setval));
    }
}


void Json::setLong(const char* path, long setval)
{
    setLong(_root, path, setval);
}


void Json::setLong(RefPtr<Value>& value, const char* path, long setval)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        throw std::runtime_error("Json::setString: Key is empty.");
    }
    if (!value)
    {
        value = RefPtr<Value>(new Value(RefPtr<Object>(new Object)));
    }
    if (value->type() != OBJECT)
    {
        throw std::runtime_error("Json::setString: Value is not an object.");
    }
    if (next)
    {
        setLong(value->object()->get(key.c_str()), next, setval);
    }
    else
    {
        value->object()->get(key.c_str()) = RefPtr<Value>(new Value(setval));
    }
}


void Json::setInt(const char* path, int setval)
{
    setInt(_root, path, setval);
}


void Json::setInt(RefPtr<Value>& value, const char* path, int setval)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        throw std::runtime_error("Json::setString: Key is empty.");
    }
    if (!value)
    {
        value = RefPtr<Value>(new Value(RefPtr<Object>(new Object)));
    }
    if (value->type() != OBJECT)
    {
        throw std::runtime_error("Json::setString: Value is not an object.");
    }
    if (next)
    {
        setInt(value->object()->get(key.c_str()), next, setval);
    }
    else
    {
        value->object()->get(key.c_str()) = RefPtr<Value>(new Value(setval));
    }
}


void Json::setBoolean(const char* path, bool setval)
{
    setBoolean(_root, path, setval);
}


void Json::setBoolean(RefPtr<Value>& value, const char* path, bool setval)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        throw std::runtime_error("Json::setString: Key is empty.");
    }
    if (!value)
    {
        value = RefPtr<Value>(new Value(RefPtr<Object>(new Object)));
    }
    if (value->type() != OBJECT)
    {
        throw std::runtime_error("Json::setString: Value is not an object.");
    }
    if (next)
    {
        setBoolean(value->object()->get(key.c_str()), next, setval);
    }
    else
    {
        value->object()->get(key.c_str()) = RefPtr<Value>(new Value(setval));
    }
}


Json::Array& Json::addArray(const char* path)
{
    return addArray(_root, path);
}


Json::Array& Json::addArray(RefPtr<Value>& value, const char* path)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        throw std::runtime_error("Json::addArray: Key is empty.");
    }
    if (!value)
    {
        value = RefPtr<Value>(new Value(RefPtr<Object>(new Object)));
    }
    if (value->type() != OBJECT)
    {
        throw std::runtime_error("Json::addArray: Value is not an object.");
    }
    RefPtr<Value>& value2 = value->object()->get(key.c_str());
    if (next)
    {
        return addArray(value2, next);
    }
    if (!value2)
    {
        value2 = RefPtr<Value>(new Value(ARRAY));
    }
    if (value2->type() != ARRAY)
    {
        throw std::runtime_error("Json::addArray: Value is not an array.");
    }
    return value2->array();
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


const RefPtr<Json::Value>& Json::Object::get(const char* key) const
{
    for (MemberArray::size_type index = 0; index < _members.size(); index++)
    {
        if (_members[index]->key() == key)
        {
            return _members[index]->value();
        }
    }
    return s_NullValue;
}


RefPtr<Json::Value>& Json::Object::get(const char* key)
{
    for (MemberArray::size_type index = 0; index < _members.size(); index++)
    {
        if (_members[index]->key() == key)
        {
            return _members[index]->value();
        }
    }
    RefPtr<Member> member(new Member(Glib::ustring(key)));
    _members.push_back(member);
    return member->value();
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


Json::Value::Value(Type type)
    : _type(type)
{
}


Json::Value::Value(Type type, const char* value)
    : _type(type)
    , _string(value ? value : "")
{
}


Json::Value::Value(const Glib::ustring& value)
    : _type(STRING)
    , _string(value)
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


Json::Value::Value(int value)
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


const Glib::ustring& Json::Value::string() const
{
    if (_type == STRING || _type == NUMBER || _type == BOOLEAN)
    {
        return _string;
    }
    else
    {
        throw std::runtime_error("Json::Value::string: Bad type.");
    }
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
    if (_type == NUMBER)
    {
        return strtol(_string.c_str(), NULL, 10);
    }
    else
    {
        throw std::runtime_error("Json::Value::integer: Bad type.");
    }
}


double Json::Value::floatingPoint() const
{
    if (_type == NUMBER)
    {
        return strtod(_string.c_str(), NULL);
    }
    else
    {
        throw std::runtime_error("Json::Value::floatingPoint: Bad type.");
    }
}


bool Json::Value::boolean() const
{
    if (_type == BOOLEAN)
    {
        if (_string == "true")
        {
            return true;
        }
        else if (_string == "false")
        {
            return false;
        }
        else
        {
            throw std::runtime_error("Json::Value::boolean: Bad value.");
        }
    }
    else
    {
        throw std::runtime_error("Json::Value::boolean: Bad type.");
    }
}


const RefPtr<Json::Object>& Json::Value::object() const
{
    if (_type == OBJECT)
    {
        return _object;
    }
    else
    {
        throw std::runtime_error("Json::Value::object: Bad type.");
    }
}


RefPtr<Json::Object>& Json::Value::object()
{
    if (_type == OBJECT)
    {
        return _object;
    }
    else
    {
        throw std::runtime_error("Json::Value::object: Bad type.");
    }
}


const Json::Array& Json::Value::array() const
{
    if (_type == ARRAY)
    {
        return _array;
    }
    else
    {
        throw std::runtime_error("Json::Value::array: Bad type.");
    }
}


Json::Array& Json::Value::array()
{
    if (_type == ARRAY)
    {
        return _array;
    }
    else
    {
        throw std::runtime_error("Json::Value::array: Bad type.");
    }
}


//////////////////////////////////////////////////////////////////////
//
// M E M B E R
//
//////////////////////////////////////////////////////////////////////


Json::Member::Member(const Glib::ustring& key)
    : _key(key)
    , _value()
{
}


Json::Member::Member(const Glib::ustring& key, const RefPtr<Value>& value)
    : _key(key)
    , _value(value)
{
}


Json::Member::~Member()
{
}
