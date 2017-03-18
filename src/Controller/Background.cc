// Copyright (C) 2012-2017 Hideaki Narita


#include "Model/ThreadNameMap.h"
#include "Background.h"


using namespace hnrt;


Background::Background(const char* name)
{
    ThreadNameMap::instance().add(name);
}


Background::~Background()
{
    ThreadNameMap::instance().remove();
}
