// Copyright (C) 2017 Hideaki Narita


#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include "JsonParser.h"
#include "JsonWriter.h"


using namespace hnrt;


static const RefPtr<Json::Value> s_NullValue;


static inline Glib::ustring GetKey(const char* path, const char** next)
{
    Glib::ustring key;
    if (path)
    {
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


bool Json::get(const char* path, Glib::ustring& retval) const
{
    if (_root)
    {
        return _root->get(path, retval);
    }
    else
    {
        return false;
    }
}


bool Json::get(const char* path, long& retval) const
{
    if (_root)
    {
        return _root->get(path, retval);
    }
    else
    {
        return false;
    }
}


bool Json::get(const char* path, int& retval) const
{
    if (_root)
    {
        return _root->get(path, retval);
    }
    else
    {
        return false;
    }
}


bool Json::get(const char* path, bool& retval) const
{
    if (_root)
    {
        return _root->get(path, retval);
    }
    else
    {
        return false;
    }
}


bool Json::get(const char* path, const sigc::slot1<void, const RefPtr<Value>&>& slot) const
{
    if (_root)
    {
        return _root->get(path, slot);
    }
    else
    {
        return false;
    }
}


void Json::set(const char* path, const Glib::ustring& setval)
{
    if (!_root)
    {
        _root = RefPtr<Value>(new Value);
    }
    _root->set(path, setval);
}


void Json::set(const char* path, long setval)
{
    if (!_root)
    {
        _root = RefPtr<Value>(new Value);
    }
    _root->set(path, setval);
}


void Json::set(const char* path, int setval)
{
    if (!_root)
    {
        _root = RefPtr<Value>(new Value);
    }
    _root->set(path, setval);
}


void Json::set(const char* path, bool setval)
{
    if (!_root)
    {
        _root = RefPtr<Value>(new Value);
    }
    _root->set(path, setval);
}


Json::Array& Json::setArray(const char* path)
{
    if (!_root)
    {
        _root = RefPtr<Value>(new Value);
    }
    return _root->setArray(path);
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
        const RefPtr<Member>& member = _members[index];
        if (member->key() == key)
        {
            return member->value();
        }
    }
    return s_NullValue;
}


RefPtr<Json::Value>& Json::Object::get(const char* key)
{
    for (MemberArray::size_type index = 0; index < _members.size(); index++)
    {
        RefPtr<Member>& member = _members[index];
        if (member->key() == key)
        {
            return member->value();
        }
    }
    add(key);
    return _members.back()->value();
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


bool Json::Value::get(const char* path, Glib::ustring& retval) const
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (!key.empty())
    {
        if (_type == OBJECT)
        {
            if (_object)
            {
                RefPtr<Value> value = _object->get(key.c_str());
                if (value)
                {
                    if (next)
                    {
                        return value->get(next, retval);
                    }
                    else if (value->_type == STRING)
                    {
                        retval = value->_string;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


bool Json::Value::get(const char* path, long& retval) const
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (!key.empty())
    {
        if (_type == OBJECT)
        {
            if (_object)
            {
                RefPtr<Value> value = _object->get(key.c_str());
                if (value)
                {
                    if (next)
                    {
                        return value->get(next, retval);
                    }
                    else if (value->_type == NUMBER)
                    {
                        retval = value->integer();
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


bool Json::Value::get(const char* path, int& retval) const
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (!key.empty())
    {
        if (_type == OBJECT)
        {
            if (_object)
            {
                RefPtr<Value> value = _object->get(key.c_str());
                if (value)
                {
                    if (next)
                    {
                        return value->get(next, retval);
                    }
                    else if (value->_type == NUMBER)
                    {
                        retval = (int)value->integer();
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


bool Json::Value::get(const char* path, bool& retval) const
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (!key.empty())
    {
        if (_type == OBJECT)
        {
            if (_object)
            {
                RefPtr<Value> value = _object->get(key.c_str());
                if (value)
                {
                    if (next)
                    {
                        return value->get(next, retval);
                    }
                    else if (value->_type == BOOLEAN)
                    {
                        retval = value->boolean();
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


bool Json::Value::get(const char* path, const sigc::slot1<void, const RefPtr<Value>&>& slot) const
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (!key.empty())
    {
        if (_type == OBJECT)
        {
            if (_object)
            {
                RefPtr<Value> value = _object->get(key.c_str());
                if (value)
                {
                    if (next)
                    {
                        return value->get(next, slot);
                    }
                    else if (value->_type == ARRAY)
                    {
                        const Json::Array& array = value->_array;
                        for (Json::Array::size_type index = 0; index < array.size(); index++)
                        {
                            slot(array[index]);
                        }
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


void Json::Value::set(const char* path, const Glib::ustring& setval)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        _type = STRING;
        _string = setval;
    }
    else
    {
        if (_type != OBJECT || !_object)
        {
            _type = OBJECT;
            _object = RefPtr<Object>(new Object);
        }
        RefPtr<Value>& value = _object->get(key.c_str());
        if (next)
        {
            value->set(next, setval);
        }
        else
        {
            value = RefPtr<Value>(new Value(setval));
        }
    }
}


void Json::Value::set(const char* path, long setval)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        _type = NUMBER;
        _string = Glib::ustring::compose("%1", setval);
    }
    else
    {
        if (_type != OBJECT || !_object)
        {
            _type = OBJECT;
            _object = RefPtr<Object>(new Object);
        }
        RefPtr<Value>& value = _object->get(key.c_str());
        if (next)
        {
            value->set(next, setval);
        }
        else
        {
            value = RefPtr<Value>(new Value(setval));
        }
    }
}


void Json::Value::set(const char* path, int setval)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        _type = NUMBER;
        _string = Glib::ustring::compose("%1", setval);
    }
    else
    {
        if (_type != OBJECT || !_object)
        {
            _type = OBJECT;
            _object = RefPtr<Object>(new Object);
        }
        RefPtr<Value>& value = _object->get(key.c_str());
        if (next)
        {
            value->set(next, setval);
        }
        else
        {
            value = RefPtr<Value>(new Value(setval));
        }
    }
}


void Json::Value::set(const char* path, bool setval)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        _type = BOOLEAN;
        _string = setval ? "true" : "false";
    }
    else
    {
        if (_type != OBJECT || !_object)
        {
            _type = OBJECT;
            _object = RefPtr<Object>(new Object);
        }
        RefPtr<Value>& value = _object->get(key.c_str());
        if (next)
        {
            value->set(next, setval);
        }
        else
        {
            value = RefPtr<Value>(new Value(setval));
        }
    }
}


Json::Array& Json::Value::setArray(const char* path)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        if (_type != ARRAY)
        {
            _type = ARRAY;
        }
        return _array;
    }
    else
    {
        if (_type != OBJECT || !_object)
        {
            _type = OBJECT;
            _object = RefPtr<Object>(new Object);
        }
        RefPtr<Value>& value = _object->get(key.c_str());
        if (next)
        {
            return value->setArray(next);
        }
        else
        {
            if (value->_type != ARRAY)
            {
                value->_type = ARRAY;
            }
            return value->_array;
        }
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
