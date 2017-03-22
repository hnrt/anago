// Copyright (C) 2012-2017 Hideaki Narita


#include "XenServer/Host.h"
#include "HostNotebook.h"
#include "NoContentsNotebook.h"
#include "NotebookFactory.h"


using namespace hnrt;


RefPtr<Notebook> NotebookFactory::create(const RefPtr<XenObject>& object)
{
    switch (object->getType())
    {
    case XenObject::HOST:
        return HostNotebook::create(RefPtr<Host>::castStatic(object));
    case XenObject::VM:
    case XenObject::SR:
    case XenObject::NETWORK:
    default:
        return NoContentsNotebook::create();
    }
}
