// Copyright (C) 2012-2017 Hideaki Narita


#include "ModelImpl.h"


using namespace hnrt;


ModelImpl::ModelImpl()
    : _log(Logger::instance())
{
    _log.trace("ModelImpl::ctor");
}


ModelImpl::~ModelImpl()
{
    _log.trace("ModelImpl::dtor");
}
