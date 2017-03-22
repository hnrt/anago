// Copyright (C) 2012-2017 Hideaki Narita


#include "Logger/Trace.h"
#include "XenServer/Session.h"
#include "XenServer/XenObject.h"
#include "Notebook.h"
#include "NotebookStore.h"


using namespace hnrt;


NotebookStore::NotebookStore()
{
    Trace trace("NotebookStore::ctor");
}


NotebookStore::~NotebookStore()
{
    Trace trace("NotebookStore::dtor");
    trace.put("size=%zu", size());
}


RefPtr<Notebook> NotebookStore::get(const RefPtr<XenObject>& node)
{
    ConstIter iter = find(node);
    return iter != end() ? iter->second : RefPtr<Notebook>();
}


RefPtr<Notebook> NotebookStore::set(const RefPtr<XenObject>& node, const RefPtr<Notebook>& notebook)
{
    RefPtr<Notebook> old;
    Iter iter = find(node);
    if (iter != end())
    {
        old = iter->second;
        iter->second = notebook;
    }
    else
    {
        insert(Entry(node, notebook));
    }
    return old;
}


RefPtr<Notebook> NotebookStore::remove(const RefPtr<XenObject>& node)
{
    RefPtr<Notebook> old;
    Iter iter = find(node);
    if (iter != end())
    {
        old = iter->second;
        erase(iter);
    }
    return old;
}
