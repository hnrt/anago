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

        JsonWriter(const JsonWriter&);
        void operator =(const JsonWriter&);
        void write(const RefPtr<Json::Object>&, int);
        void write(const Json::Array&, int);
        void write(const RefPtr<Json::Member>&, int);
        void write(const RefPtr<Json::Value>&, int);
        void writeFalse(const RefPtr<Json::Value>&, int);
        void writeNull(const RefPtr<Json::Value>&, int);
        void writeTrue(const RefPtr<Json::Value>&, int);
        void writeString(const RefPtr<Json::Value>&, int);
        void writeNumber(const RefPtr<Json::Value>&, int);
        void writeObject(const RefPtr<Json::Value>&, int);
        void writeArray(const RefPtr<Json::Value>&, int);
        void indent(int);

        typedef void (JsonWriter::*ValueWrite)(const RefPtr<Json::Value>&, int);
        typedef std::map<Json::Type, ValueWrite> ValueWriteMap;

        FILE* _fp;
        const Json& _doc;
        ValueWriteMap _map;
    };
}


#endif //!HNRT_JSON_WRITER_H
