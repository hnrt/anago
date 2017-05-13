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

        JsonWriter(FILE*);
        void write(const RefPtr<Json>&);

    private:

        typedef void (JsonWriter::*WriteValue)(const RefPtr<Json>&, int);
        typedef std::map<Json::Type, WriteValue> WriteValueMap;

        JsonWriter(const JsonWriter&);
        void operator =(const JsonWriter&);
        void writeValue(const RefPtr<Json>&, int);
        void writeNull(const RefPtr<Json>&, int);
        void writeBoolean(const RefPtr<Json>&, int);
        void writeString(const RefPtr<Json>&, int);
        void writeNumber(const RefPtr<Json>&, int);
        void writeObject(const RefPtr<Json>&, int);
        void writeMember(const RefPtr<Json::Member>&, int);
        void writeArray(const RefPtr<Json>&, int);
        void indent(int);

        FILE* _fp;
        WriteValueMap _map;
    };
}


#endif //!HNRT_JSON_WRITER_H
