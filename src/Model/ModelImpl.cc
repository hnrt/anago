// Copyright (C) 2012-2017 Hideaki Narita


#include "Logger/Trace.h"
#include "ModelImpl.h"


using namespace hnrt;


ModelImpl::ModelImpl()
{
    Trace trace(__PRETTY_FUNCTION__);
}


ModelImpl::~ModelImpl()
{
    Trace trace(__PRETTY_FUNCTION__);
}
