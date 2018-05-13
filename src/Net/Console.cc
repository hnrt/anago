// Copyright (C) 2012-2018 Hideaki Narita


#include "ConsoleImpl.h"


using namespace hnrt;


RefPtr<Console> Console::create(ConsoleView& view)
{
    return RefPtr<Console>(new ConsoleImpl(view));
}


Console::Console()
{
}


Console::~Console()
{
}
