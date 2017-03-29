// Copyright (C) 2012-2017 Hideaki Narita


#include "App/Constants.h"
#include "App/Version.h"
#include "XenServer/Host.h"
#include "XenServer/Network.h"
#include "XenServer/StorageRepository.h"
#include "HostNotebook.h"
#include "NetworkNotebook.h"
#include "NoContentsNotebook.h"
#include "NotebookFactory.h"
#include "StorageRepositoryNotebook.h"


using namespace hnrt;


RefPtr<Notebook> NotebookFactory::create()
{
    return NoContentsNotebook::create(Glib::ustring::compose("%1 version %2\n%3", APPDISPNAME, VERSION, COPYRIGHT));
}


RefPtr<Notebook> NotebookFactory::create(XenObject& object)
{
    switch (object.getType())
    {
    case XenObject::HOST:
        return HostNotebook::create(static_cast<Host&>(object));
    case XenObject::SR:
        return StorageRepositoryNotebook::create(static_cast<StorageRepository&>(object));
    case XenObject::NETWORK:
        return NetworkNotebook::create(static_cast<Network&>(object));
    case XenObject::VM:
    default:
        return create();
    }
}
