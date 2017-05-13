// Copyright (C) 2017 Hideaki Narita


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include "JsonParser.h"
#include "JsonWriter.h"


using namespace hnrt;


static const RefPtr<Json> s_NullValue;


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


RefPtr<Json> Json::load(FILE* fp)
{
    JsonLexer lexer(fp);
    JsonParser parser(lexer);
    return parser.run();
}


Json::Json()
    : _type(NULLVALUE)
{
}


Json::Json(Type type)
    : _type(type)
{
}


Json::Json(Type type, const char* value)
    : _type(type)
    , _string(value ? value : "")
{
}


Json::Json(const Glib::ustring& value)
    : _type(STRING)
    , _string(value)
{
}


Json::Json(const char* value)
    : _type(STRING)
    , _string(value ? value : "")
{
}


Json::Json(long value)
    : _type(NUMBER)
    , _string(Glib::ustring::compose("%1", value))
{
}


Json::Json(int value)
    : _type(NUMBER)
    , _string(Glib::ustring::compose("%1", value))
{
}


Json::Json(double value)
    : _type(NUMBER)
    , _string(Glib::ustring::compose("%1", value))
{
}


Json::Json(bool value)
    : _type(BOOLEAN)
    , _string(value ? "true" : "false")
{
}


Json::~Json()
{
}


void Json::save(FILE* fp)
{
    JsonWriter writer(fp);
    writer.write(RefPtr<Json>(this, 1));
}


void Json::add(const RefPtr<Member>& member)
{
    if (_type == OBJECT)
    {
        _members.push_back(member);
    }
    else
    {
        throw std::runtime_error("Json::add(OBJECT): Bad type.");
    }
}


void Json::add(const RefPtr<Json>& element)
{
    if (_type == ARRAY)
    {
        _array.push_back(element);
    }
    else
    {
        throw std::runtime_error("Json::add(ARRAY): Bad type.");
    }
}


const Glib::ustring& Json::string() const
{
    if (_type == STRING || _type == NUMBER || _type == BOOLEAN)
    {
        return _string;
    }
    else
    {
        throw std::runtime_error("Json::string: Bad type.");
    }
}


bool Json::isInteger() const
{
    return _type == NUMBER && _string.find('.') == Glib::ustring::npos;
}


bool Json::isFloatingPoint() const
{
    return _type == NUMBER && _string.find('.') != Glib::ustring::npos;
}


long Json::integer() const
{
    if (_type == NUMBER)
    {
        return strtol(_string.c_str(), NULL, 10);
    }
    else
    {
        throw std::runtime_error("Json::integer: Bad type.");
    }
}


double Json::floatingPoint() const
{
    if (_type == NUMBER)
    {
        return strtod(_string.c_str(), NULL);
    }
    else
    {
        throw std::runtime_error("Json::floatingPoint: Bad type.");
    }
}


bool Json::boolean() const
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
            throw std::runtime_error("Json::boolean: Bad value.");
        }
    }
    else
    {
        throw std::runtime_error("Json::boolean: Bad type.");
    }
}


const Json::MemberArray& Json::members() const
{
    if (_type == OBJECT)
    {
        return _members;
    }
    else
    {
        throw std::runtime_error("Json::members: Bad type.");
    }
}


const Json::Array& Json::array() const
{
    if (_type == ARRAY)
    {
        return _array;
    }
    else
    {
        throw std::runtime_error("Json::array: Bad type.");
    }
}


Json::Array& Json::array()
{
    if (_type == ARRAY)
    {
        return _array;
    }
    else
    {
        throw std::runtime_error("Json::array: Bad type.");
    }
}


const RefPtr<Json>& Json::find(const Glib::ustring& key) const
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


RefPtr<Json>& Json::find(const Glib::ustring& key)
{
    for (MemberArray::size_type index = 0; index < _members.size(); index++)
    {
        RefPtr<Member>& member = _members[index];
        if (member->key() == key)
        {
            return member->value();
        }
    }
    _members.push_back(Member::create(key));
    return _members.back()->value();
}


bool Json::get(const char* path, Glib::ustring& retval) const
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        if (_type == STRING)
        {
            retval = _string;
            return true;
        }
    }
    else if (_type == OBJECT)
    {
        RefPtr<Json> value = find(key);
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
    return false;
}


bool Json::get(const char* path, long& retval) const
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        if (_type == NUMBER)
        {
            retval = strtol(_string.c_str(), NULL, 10);
            return true;
        }
    }
    else if (_type == OBJECT)
    {
        RefPtr<Json> value = find(key);
        if (value)
        {
            if (next)
            {
                return value->get(next, retval);
            }
            else if (value->_type == NUMBER)
            {
                retval = strtol(value->_string.c_str(), NULL, 10);
                return true;
            }
        }
    }
    return false;
}


bool Json::get(const char* path, int& retval) const
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        if (_type == NUMBER)
        {
            retval = (int)strtol(_string.c_str(), NULL, 10);
            return true;
        }
    }
    else if (_type == OBJECT)
    {
        RefPtr<Json> value = find(key);
        if (value)
        {
            if (next)
            {
                return value->get(next, retval);
            }
            else if (value->_type == NUMBER)
            {
                retval = (int)strtol(value->_string.c_str(), NULL, 10);
                return true;
            }
        }
    }
    return false;
}


bool Json::get(const char* path, double& retval) const
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        if (_type == NUMBER)
        {
            retval = strtod(_string.c_str(), NULL);
            return true;
        }
    }
    else if (_type == OBJECT)
    {
        RefPtr<Json> value = find(key);
        if (value)
        {
            if (next)
            {
                return value->get(next, retval);
            }
            else if (value->_type == NUMBER)
            {
                retval = strtod(value->_string.c_str(), NULL);
                return true;
            }
        }
    }
    return false;
}


bool Json::get(const char* path, bool& retval) const
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        if (_type == BOOLEAN)
        {
            if (_string == "true")
            {
                retval = true;
                return true;
            }
            else if (_string == "false")
            {
                retval = false;
                return true;
            }
        }
    }
    else if (_type == OBJECT)
    {
        RefPtr<Json> value = find(key);
        if (value)
        {
            if (next)
            {
                return value->get(next, retval);
            }
            else if (value->_type == BOOLEAN)
            {
                if (value->_string == "true")
                {
                    retval = true;
                    return true;
                }
                else if (value->_string == "false")
                {
                    retval = false;
                    return true;
                }
            }
        }
    }
    return false;
}


bool Json::get(const char* path, const sigc::slot1<void, const RefPtr<Json>&>& slot) const
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        if (_type == ARRAY)
        {
            for (Json::Array::size_type index = 0; index < _array.size(); index++)
            {
                slot(_array[index]);
            }
            return true;
        }
    }
    else if (_type == OBJECT)
    {
        RefPtr<Json> value = find(key);
        if (value)
        {
            if (next)
            {
                return value->get(next, slot);
            }
            else if (value->_type == ARRAY)
            {
                for (Json::Array::size_type index = 0; index < value->_array.size(); index++)
                {
                    slot(value->_array[index]);
                }
                return true;
            }
        }
    }
    return false;
}


void Json::set(const char* path, const Glib::ustring& setval)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        _type = STRING;
        _string = setval;
    }
    else if (_type == OBJECT)
    {
        RefPtr<Json>& value = find(key);
        if (next)
        {
            if (!value)
            {
                value = Json::create(OBJECT);
            }
            value->set(next, setval);
        }
        else
        {
            value = Json::create(setval);
        }
    }
    else
    {
        throw std::runtime_error("Json::set(string): Bad type.");
    }
}


void Json::set(const char* path, long setval)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        _type = NUMBER;
        _string = Glib::ustring::compose("%1", setval);
    }
    else if (_type == OBJECT)
    {
        RefPtr<Json>& value = find(key);
        if (next)
        {
            if (!value)
            {
                value = Json::create(OBJECT);
            }
            value->set(next, setval);
        }
        else
        {
            value = Json::create(setval);
        }
    }
    else
    {
        throw std::runtime_error("Json::set(long): Bad type.");
    }
}


void Json::set(const char* path, int setval)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        _type = NUMBER;
        _string = Glib::ustring::compose("%1", setval);
    }
    else if (_type == OBJECT)
    {
        RefPtr<Json>& value = find(key);
        if (next)
        {
            if (!value)
            {
                value = Json::create(OBJECT);
            }
            value->set(next, setval);
        }
        else
        {
            value = Json::create(setval);
        }
    }
    else
    {
        throw std::runtime_error("Json::set(int): Bad type.");
    }
}


void Json::set(const char* path, double setval)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        _type = NUMBER;
        _string = Glib::ustring::compose("%1", setval);
    }
    else if (_type == OBJECT)
    {
        RefPtr<Json>& value = find(key);
        if (next)
        {
            if (!value)
            {
                value = Json::create(OBJECT);
            }
            value->set(next, setval);
        }
        else
        {
            value = Json::create(setval);
        }
    }
    else
    {
        throw std::runtime_error("Json::set(double): Bad type.");
    }
}


void Json::set(const char* path, bool setval)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        _type = BOOLEAN;
        _string = setval ? "true" : "false";
    }
    else if (_type == OBJECT)
    {
        RefPtr<Json>& value = find(key);
        if (next)
        {
            if (!value)
            {
                value = Json::create(OBJECT);
            }
            value->set(next, setval);
        }
        else
        {
            value = Json::create(setval);
        }
    }
    else
    {
        throw std::runtime_error("Json::set(bool): Bad type.");
    }
}


Json::Array& Json::setArray(const char* path)
{
    const char* next = NULL;
    Glib::ustring key = GetKey(path, &next);
    if (key.empty())
    {
        _type = ARRAY;
        return _array;
    }
    else if (_type == OBJECT)
    {
        RefPtr<Json>& value = find(key);
        if (next)
        {
            if (!value)
            {
                value = Json::create(OBJECT);
            }
            return value->setArray(next);
        }
        else
        {
            value = Json::create(ARRAY);
            return value->_array;
        }
    }
    else
    {
        throw std::runtime_error("Json::setArray: Bad type.");
    }
}
