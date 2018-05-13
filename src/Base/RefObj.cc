// Copyright (C) 2012-2018 Hideaki Narita


#include "Atomic.h"
#include "RefObj.h"
//#include "Logger/Logger.h"


using namespace hnrt;


RefObj::RefObj()
    : _refCount(1)
{
}


RefObj::~RefObj()
{
}


void RefObj::incRef() const
{
    int count = InterlockedIncrement(&const_cast<RefObj*>(this)->_refCount);
    //Logger::instance().trace("RefObj@%zx::incRef: %d", this, count);
    (void)count;
}


void RefObj::decRef() const
{
    int count = InterlockedDecrement(&const_cast<RefObj*>(this)->_refCount);
    //Logger::instance().trace("RefObj@%zx::decRef: %d", this, count);
    if (!count)
    {
        delete this;
    }
}
