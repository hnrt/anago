// Copyright (C) 2017 Hideaki Narita


#include "Env.h"
#include "Locale.h"
#include "Process.h"


using namespace hnrt;


void Env::init()
{
    Locale::init();
    Process::init();
}


void Env::fini()
{
    Locale::fini();
    Process::fini();
}
