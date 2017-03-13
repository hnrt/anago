// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_JSON_WRITER_H
#define HNRT_JSON_WRITER_H


#include <stdio.h>
#include <map>
#include "Json.h"


namespace hnrt
{
    class JsonWriter
    {
    public:

        JsonWriter(FILE*, const Json&);
        void write();

    private:

        typedef void (JsonWriter::*WriteValue)(const RefPtr<Json::Value>&, int);
        typedef std::map<Json::Type, WriteValue> WriteValueMap;

        JsonWriter(const JsonWriter&);
        void operator =(const JsonWriter&);
        void writeValue(const RefPtr<Json::Value>&, int);
        void writeNull(const RefPtr<Json::Value>&, int);
        void writeBoolean(const RefPtr<Json::Value>&, int);
        void writeString(const RefPtr<Json::Value>&, int);
        void writeNumber(const RefPtr<Json::Value>&, int);
        void writeObject(const RefPtr<Json::Value>&, int);
        void writeMember(const RefPtr<Json::Member>&, int);
        void writeArray(const RefPtr<Json::Value>&, int);
        void indent(int);

        FILE* _fp;
        const Json& _doc;
        WriteValueMap _map;
    };
}


#endif //!HNRT_JSON_WRITER_H
