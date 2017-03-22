// Copyright (C) 2012-2017 Hideaki Narita


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
        : public std::map<RefPtr<XenObject>, Glib::RefPtr<Notebook> >
    {
    public:

        typedef std::map<RefPtr<XenObject>, Glib::RefPtr<Notebook> > Super;
        typedef std::map<RefPtr<XenObject>, Glib::RefPtr<Notebook> >::iterator Iter;
        typedef std::map<RefPtr<XenObject>, Glib::RefPtr<Notebook> >::const_iterator ConstIter;
        typedef std::pair<RefPtr<XenObject>, Glib::RefPtr<Notebook> > Entry;

        NotebookStore();
        ~NotebookStore();
        Glib::RefPtr<Notebook> get(const RefPtr<XenObject>&);
        Glib::RefPtr<Notebook> set(const RefPtr<XenObject>&, const Glib::RefPtr<Notebook>&);
        Glib::RefPtr<Notebook> remove(const RefPtr<XenObject>&);

    private:

        NotebookStore(const NotebookStore&);
        void operator =(const NotebookStore&);
    };
}


#endif //!HNRT_NOTEBOOKSTORE_H
