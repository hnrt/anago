// Copyright (C) 2012-2017 Hideaki Narita


#include "XenServer/XenObject.h"
#include "NoContentsNotebook.h"
#include "NotebookFactory.h"


using namespace hnrt;


Glib::RefPtr<Notebook> NotebookFactory::create(const RefPtr<XenObject>& object)
{
    switch (object->getType())
    {
    case XenObject::HOST:
    case XenObject::VM:
    case XenObject::SR:
    case XenObject::NETWORK:
    default:
        return NoContentsNotebook::create();
    }
}
