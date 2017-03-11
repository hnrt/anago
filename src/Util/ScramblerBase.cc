// Copyright (C) 2012-2017 Hideaki Narita


#include <stdlib.h>
#include <string.h>
#include "ScramblerBase.h"


using namespace hnrt;


ScramblerBase::ScramblerBase(size_t n, size_t extra)
    : _value(new unsigned char[n + extra])
    , _length(n)
{
}


ScramblerBase::~ScramblerBase()
{
    free();
}


void ScramblerBase::clear()
{
    free();
    _value = NULL;
    _length = 0;
}


void ScramblerBase::free()
{
    if (_value)
    {
        memset(_value, 0, _length);
        delete[] _value;
    }
}
