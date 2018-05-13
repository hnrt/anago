// Copyright (C) 2018 Hideaki Narita


#include "JsonWriter.h"


using namespace hnrt;


JsonWriter::JsonWriter(FILE* fp)
    : _fp(fp)
{
    _map.insert(WriteValueMap::value_type(Json::NULLVALUE, &JsonWriter::writeNull));
    _map.insert(WriteValueMap::value_type(Json::BOOLEAN, &JsonWriter::writeBoolean));
    _map.insert(WriteValueMap::value_type(Json::STRING, &JsonWriter::writeString));
    _map.insert(WriteValueMap::value_type(Json::NUMBER, &JsonWriter::writeNumber));
    _map.insert(WriteValueMap::value_type(Json::OBJECT, &JsonWriter::writeObject));
    _map.insert(WriteValueMap::value_type(Json::ARRAY, &JsonWriter::writeArray));
}


void JsonWriter::write(const RefPtr<Json>& doc)
{
    writeValue(doc, 0);
    putc('\n', _fp);
}


void JsonWriter::writeValue(const RefPtr<Json>& value, int level)
{
    WriteValueMap::const_iterator iter = _map.find(value->type());
    if (iter != _map.end())
    {
        WriteValue func = iter->second;
        (this->*func)(value, level);
    }
    else
    {
        throw Glib::ustring::compose("Bad value type: %1", (int)value->type());
    }
}


void JsonWriter::writeNull(const RefPtr<Json>& value, int level)
{
    fputs("null", _fp);
}


void JsonWriter::writeBoolean(const RefPtr<Json>& value, int level)
{
    fputs(value->boolean() ? "true" : "false", _fp);
}


void JsonWriter::writeString(const RefPtr<Json>& value, int level)
{
    putc('\"', _fp);
    for (const char* s = value->string().c_str(); *s; s++)
    {
        int c = *s & 0xff;
        if (c == '\"' || c == '\\')
        {
            putc('\\', _fp);
            putc(c, _fp);
        }
        else if (c == '\b')
        {
            putc('\\', _fp);
            putc('b', _fp);
        }
        else if (c == '\f')
        {
            putc('\\', _fp);
            putc('f', _fp);
        }
        else if (c == '\n')
        {
            putc('\\', _fp);
            putc('n', _fp);
        }
        else if (c == '\r')
        {
            putc('\\', _fp);
            putc('r', _fp);
        }
        else if (c == '\t')
        {
            putc('\\', _fp);
            putc('t', _fp);
        }
        else
        {
            putc(c, _fp);
        }
    }
    putc('\"', _fp);
}


void JsonWriter::writeNumber(const RefPtr<Json>& value, int level)
{
    fputs(value->string().c_str(), _fp);
}


void JsonWriter::writeObject(const RefPtr<Json>& value, int level)
{
    putc(Json::BEGIN_OBJECT, _fp);
    const Json::MemberArray& members = value->members();
    if (members.size())
    {
        putc('\n', _fp);
        indent(level + 1);
        writeMember(members[0], level + 1);
        for (Json::MemberArray::size_type index = 1; index < members.size(); index++)
        {
            putc(Json::VALUE_SEPARATOR, _fp);
            putc('\n', _fp);
            indent(level + 1);
            writeMember(members[index], level + 1);
        }
    }
    putc('\n', _fp);
    indent(level);
    putc(Json::END_OBJECT, _fp);
}


void JsonWriter::writeMember(const RefPtr<Json::Member>& member, int level)
{
    putc('\"', _fp);
    fputs(member->key().c_str(), _fp);
    putc('\"', _fp);
    putc(Json::NAME_SEPARATOR, _fp);
    putc(' ', _fp);
    writeValue(member->value(), level);
}


void JsonWriter::writeArray(const RefPtr<Json>& value, int level)
{
    putc(Json::BEGIN_ARRAY, _fp);
    const Json::Array& array = value->array();
    if (array.size())
    {
        putc('\n', _fp);
        indent(level + 1);
        writeValue(array[0], level + 1);
        for (Json::Array::size_type index = 1; index < array.size(); index++)
        {
            putc(Json::VALUE_SEPARATOR, _fp);
            putc('\n', _fp);
            indent(level + 1);
            writeValue(array[index], level + 1);
        }
    }
    putc('\n', _fp);
    indent(level);
    putc(Json::END_ARRAY, _fp);
}


void JsonWriter::indent(int level)
{
    for (int count = level * 4; count > 0; count--)
    {
        putc(' ', _fp);
    }
}
