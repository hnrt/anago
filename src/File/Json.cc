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
