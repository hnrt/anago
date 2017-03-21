// Copyright (C) 2012-2017 Hideaki Narita


#include "XenServer/XenObject.h"
#include "Notebook.h"
#include "NotebookStore.h"


using namespace hnrt;


NotebookStore::NotebookStore()
{
}


Glib::RefPtr<Notebook> NotebookStore::get(const RefPtr<XenObject>& node)
{
    ConstIter iter = find(node);
    return iter != end() ? iter->second : Glib::RefPtr<Notebook>();
}


Glib::RefPtr<Notebook> NotebookStore::set(const RefPtr<XenObject>& node, const Glib::RefPtr<Notebook>& notebook)
{
    Glib::RefPtr<Notebook> old;
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


Glib::RefPtr<Notebook> NotebookStore::remove(const RefPtr<XenObject>& node)
{
    Glib::RefPtr<Notebook> old;
    Iter iter = find(node);
    if (iter != end())
    {
        old = iter->second;
        erase(iter);
    }
    return old;
}
