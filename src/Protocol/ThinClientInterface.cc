// Copyright (C) 2017 Hideaki Narita


#include "ThinClientInterfaceImpl.h"


using namespace hnrt;


RefPtr<ThinClientInterface> ThinClientInterface::create()
{
    return RefPtr<ThinClientInterface>(new ThinClientInterfaceImpl());
}


ThinClientInterface::ThinClientInterface()
{
}


ThinClientInterface::~ThinClientInterface()
{
}
