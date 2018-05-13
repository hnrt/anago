// Copyright (C) 2012-2018 Hideaki Narita


#include "CifsSpec.h"


using namespace hnrt;


CifsSpec::CifsSpec()
{
}


CifsSpec::CifsSpec(const CifsSpec& other)
    : label(other.label)
    , description(other.description)
    , location(other.location)
    , username(other.username)
    , password(other.password)
{
}


void CifsSpec::operator =(const CifsSpec& rhs)
{
    label = rhs.label;
    description = rhs.description;
    location = rhs.location;
    username = rhs.username;
    password = rhs.password;
}
