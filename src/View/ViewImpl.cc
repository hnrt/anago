// Copyright (C) 2012-2017 Hideaki Narita


#include "Logger/Trace.h"
#include "ViewImpl.h"


using namespace hnrt;


ViewImpl::ViewImpl()
{
    Trace trace(__PRETTY_FUNCTION__);
}


ViewImpl::~ViewImpl()
{
    Trace trace(__PRETTY_FUNCTION__);
}
