// Copyright (C) 2017 Hideaki Narita


#include "JsonWriter.h"


using namespace hnrt;


JsonWriter::JsonWriter(FILE* fp, const Json& doc)
    : _fp(fp)
    , _doc(doc)
{
    _map.insert(WriteValueMap::value_type(Json::VALUE_FALSE, &JsonWriter::writeFalse));
    _map.insert(WriteValueMap::value_type(Json::VALUE_NULL, &JsonWriter::writeNull));
    _map.insert(WriteValueMap::value_type(Json::VALUE_TRUE, &JsonWriter::writeTrue));
    _map.insert(WriteValueMap::value_type(Json::STRING, &JsonWriter::writeString));
    _map.insert(WriteValueMap::value_type(Json::NUMBER, &JsonWriter::writeNumber));
    _map.insert(WriteValueMap::value_type(Json::OBJECT, &JsonWriter::writeObject));
    _map.insert(WriteValueMap::value_type(Json::ARRAY, &JsonWriter::writeArray));
}


void JsonWriter::write()
{
    writeValue(_doc.root(), 0);
    putc('\n', _fp);
}


void JsonWriter::writeValue(const RefPtr<Json::Value>& value, int level)
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


void JsonWriter::writeFalse(const RefPtr<Json::Value>& value, int level)
{
    fputs("false", _fp);
}


void JsonWriter::writeNull(const RefPtr<Json::Value>& value, int level)
{
    fputs("null", _fp);
}


void JsonWriter::writeTrue(const RefPtr<Json::Value>& value, int level)
{
    fputs("true", _fp);
}


void JsonWriter::writeString(const RefPtr<Json::Value>& value, int level)
{
    putc('\"', _fp);
    fputs(value->string().c_str(), _fp);
    putc('\"', _fp);
}


void JsonWriter::writeNumber(const RefPtr<Json::Value>& value, int level)
{
    fprintf(_fp, "%ld", value->number());
}


void JsonWriter::writeObject(const RefPtr<Json::Value>& value, int level)
{
    putc(Json::BEGIN_OBJECT, _fp);
    const Json::MemberArray& members = value->object()->members();
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


void JsonWriter::writeArray(const RefPtr<Json::Value>& value, int level)
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
