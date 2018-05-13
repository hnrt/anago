// Copyright (C) 2012-2018 Hideaki Narita


#include "Logger/Trace.h"
#include "XenServer/Session.h"
#include "XenServer/XenObject.h"
#include "Notebook.h"
#include "NotebookStore.h"


using namespace hnrt;


NotebookStore::NotebookStore()
{
    TRACEFUN(this, "NotebookStore::ctor");
}


NotebookStore::~NotebookStore()
{
    TRACEFUN(this, "NotebookStore::dtor");
    TRACEPUT("size=%zu", size());
}


RefPtr<Notebook> NotebookStore::get(const XenObject& node)
{
    ConstIter iter = find(&node);
    return iter != end() ? iter->second : RefPtr<Notebook>();
}


RefPtr<Notebook> NotebookStore::set(const XenObject& node, const RefPtr<Notebook>& notebook)
{
    RefPtr<Notebook> old;
    Iter iter = find(&node);
    if (iter != end())
    {
        old = iter->second;
        iter->second = notebook;
    }
    else
    {
        insert(Entry(&node, notebook));
    }
    return old;
}


RefPtr<Notebook> NotebookStore::remove(const XenObject& node)
{
    RefPtr<Notebook> old;
    Iter iter = find(&node);
    if (iter != end())
    {
        old = iter->second;
        erase(iter);
    }
    return old;
}
