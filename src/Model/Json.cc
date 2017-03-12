// Copyright (C) 2017 Hideaki Narita


#include <stdio.h>
#include "JsonParser.h"


using namespace hnrt;


Json::Json()
    : _type(UNDEFINED)
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
    if (_type == OBJECT)
    {
        write(fp, _object, 0);
        fprintf(fp, "\n");
    }
    else if (_type == ARRAY)
    {
        write(fp, _array, 0);
        fprintf(fp, "\n");
    }
    else
    {
        throw Glib::ustring("Attempted writing an empty JSON object.");
    }
}


void Json::set(const RefPtr<Object>& object)
{
    _type = OBJECT;
    _object = object;
}


Json::Array& Json::array()
{
    _type = ARRAY;
    return _array;
}


void Json::write(FILE* fp, const RefPtr<Object>& object, int level)
{
    fprintf(fp, "{");
    const MemberArray& members = object->members();
    if (members.size())
    {
        fprintf(fp, "\n");
        indent(fp, level + 1);
        write(fp, members[0], level + 1);
        for (size_t index = 1; index < members.size(); index++)
        {
            fprintf(fp, ",\n");
            indent(fp, level + 1);
            write(fp, members[index], level + 1);
        }
    }
    fprintf(fp, "\n");
    indent(fp, level);
    fprintf(fp, "}");
}


void Json::write(FILE* fp, const Array& array, int level)
{
    fprintf(fp, "[");
    if (array.size())
    {
        fprintf(fp, "\n");
        indent(fp, level + 1);
        write(fp, array[0], level + 1);
        for (size_t index = 1; index < array.size(); index++)
        {
            fprintf(fp, ",\n");
            indent(fp, level + 1);
            write(fp, array[index], level + 1);
        }
    }
    fprintf(fp, "\n");
    indent(fp, level);
    fprintf(fp, "]");
}


void Json::write(FILE* fp, const RefPtr<Member>& member, int level)
{
    fprintf(fp, "\"%s\": ", member->key().c_str());
    write(fp, member->value(), level);
}


void Json::write(FILE* fp, const RefPtr<Value>& value, int level)
{
    if (value->type() == VALUE_FALSE)
    {
        fprintf(fp, "false");
    }
    else if (value->type() == VALUE_NULL)
    {
        fprintf(fp, "null");
    }
    else if (value->type() == VALUE_TRUE)
    {
        fprintf(fp, "true");
    }
    else if (value->type() == STRING)
    {
        fprintf(fp, "\"%s\"", value->string().c_str());
    }
    else if (value->type() == NUMBER)
    {
        fprintf(fp, "%ld", value->number());
    }
    else if (value->type() == OBJECT)
    {
        write(fp, value->object(), level);
    }
    else if (value->type() == ARRAY)
    {
        write(fp, value->array(), level);
    }
    else
    {
        throw Glib::ustring::compose("Bad value type: %1", (int)value->type());
    }
}


void Json::indent(FILE* fp, int level)
{
    for (int count = level * 4; count > 0; count--)
    {
        putc(' ', fp);
    }
}
