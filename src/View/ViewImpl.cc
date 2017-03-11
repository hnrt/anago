// Copyright (C) 2012-2017 Hideaki Narita


#include "ViewImpl.h"


using namespace hnrt;


ViewImpl::ViewImpl()
    : _log(Logger::instance())
{
    _log.trace("ViewImpl::ctor");
}


ViewImpl::~ViewImpl()
{
    _log.trace("ViewImpl::dtor");
}
