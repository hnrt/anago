// Copyright (C) 2012-2017 Hideaki Narita


#include "Atomic.h"
#include "RefObj.h"


using namespace hnrt;


RefObj::RefObj()
    : _refCount(1)
{
}


RefObj::~RefObj()
{
}


void RefObj::reference() const
{
    InterlockedIncrement(&const_cast<RefObj*>(this)->_refCount);
}


void RefObj::unreference() const
{
    int retval = InterlockedDecrement(&const_cast<RefObj*>(this)->_refCount);
    if (!retval)
    {
        delete this;
    }
}
