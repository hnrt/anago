// Copyright (C) 2012-2018 Hideaki Narita


#include "App/Constants.h"
#include "App/Version.h"
#include "XenServer/Host.h"
#include "XenServer/Network.h"
#include "XenServer/StorageRepository.h"
#include "XenServer/VirtualMachine.h"
#include "HostNotebook.h"
#include "NetworkNotebook.h"
#include "NoContentsNotebook.h"
#include "NotebookFactory.h"
#include "StorageRepositoryNotebook.h"
#include "VirtualMachineNotebook.h"


using namespace hnrt;


RefPtr<Notebook> NotebookFactory::create()
{
    return NoContentsNotebook::create(Glib::ustring::compose("%1 version %2\n%3", APPDISPNAME, VERSION, COPYRIGHT));
}


RefPtr<Notebook> NotebookFactory::create(const RefPtr<XenObject>& object)
{
    switch (object->getType())
    {
    case XenObject::HOST:
        return HostNotebook::create(RefPtr<Host>::castStatic(object));
    case XenObject::VM:
        return VirtualMachineNotebook::create(RefPtr<VirtualMachine>::castStatic(object));
    case XenObject::SR:
        return StorageRepositoryNotebook::create(RefPtr<StorageRepository>::castStatic(object));
    case XenObject::NETWORK:
        return NetworkNotebook::create(RefPtr<Network>::castStatic(object));
    default:
        return create();
    }
}
