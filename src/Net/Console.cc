// Copyright (C) 2012-2017 Hideaki Narita


#include "ConsoleImpl.h"


using namespace hnrt;


RefPtr<Console> Console::create(ConsoleView& view)
{
    return ConsoleImpl::create(view);
}


Console::Console()
{
}


Console::~Console()
{
}
