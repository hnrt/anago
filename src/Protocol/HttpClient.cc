// Copyright (C) 2018 Hideaki Narita


#include "HttpClientImpl.h"


using namespace hnrt;


RefPtr<HttpClient> HttpClient::create()
{
    return RefPtr<HttpClient>(new HttpClientImpl());
}


HttpClient::HttpClient()
{
}


HttpClient::~HttpClient()
{
}
