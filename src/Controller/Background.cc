// Copyright (C) 2012-2017 Hideaki Narita


#include "Model/ThreadManager.h"
#include "Background.h"


using namespace hnrt;


Background::Background(const char* name)
{
    ThreadManager::instance().add(name);
}


Background::~Background()
{
    ThreadManager::instance().remove();
}
