// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_NOTEBOOKSTORE_H
#define HNRT_NOTEBOOKSTORE_H


#include <glibmm.h>
#include <map>
#include "Base/RefPtr.h"


namespace hnrt
{
    class Notebook;
    class XenObject;

    class NotebookStore
        : public std::map<const XenObject*, RefPtr<Notebook> >
    {
    public:

        typedef std::map<const XenObject*, RefPtr<Notebook> > Super;
        typedef std::map<const XenObject*, RefPtr<Notebook> >::iterator Iter;
        typedef std::map<const XenObject*, RefPtr<Notebook> >::const_iterator ConstIter;
        typedef std::pair<const XenObject*, RefPtr<Notebook> > Entry;

        NotebookStore();
        ~NotebookStore();
        RefPtr<Notebook> get(const XenObject&);
        RefPtr<Notebook> set(const XenObject&, const RefPtr<Notebook>&);
        RefPtr<Notebook> remove(const XenObject&);

    private:

        NotebookStore(const NotebookStore&);
        void operator =(const NotebookStore&);
    };
}


#endif //!HNRT_NOTEBOOKSTORE_H
