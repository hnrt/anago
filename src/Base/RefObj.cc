// Copyright (C) 2012-2017 Hideaki Narita


#include "Atomic.h"
#include "RefObj.h"
#include "Logger/Logger.h"


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
    int count = InterlockedIncrement(&const_cast<RefObj*>(this)->_refCount);
    Logger::instance().trace("RefObj@%zx::reference: %d", this, count);
    (void)count;
}


void RefObj::unreference() const
{
    int count = InterlockedDecrement(&const_cast<RefObj*>(this)->_refCount);
    Logger::instance().trace("RefObj@%zx::unreference: %d", this, count);
    if (!count)
    {
        delete this;
    }
}
