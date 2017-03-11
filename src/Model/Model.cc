// Copyright (C) 2012-2017 Hideaki Narita


#include "ModelImpl.h"


using namespace hnrt;


static ModelImpl* _singleton = NULL;


void Model::init()
{
    _singleton = new ModelImpl();
}


void Model::fini()
{
    delete _singleton;
}


Model& Model::instance()
{
    return *_singleton;
}
