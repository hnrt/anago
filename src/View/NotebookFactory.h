// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_NOTEBOOKFACTORY_H
#define HNRT_NOTEBOOKFACTORY_H


#include <glibmm.h>
#include "Base/RefPtr.h"


namespace hnrt
{
    class Notebook;
    class XenObject;

    class NotebookFactory
    {
    public:

        static RefPtr<Notebook> create();
        static RefPtr<Notebook> create(const RefPtr<XenObject>&);
    };
}


#endif //!HNRT_NOTEBOOKFACTORY_H
