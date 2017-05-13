// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_NOTEBOOK_H
#define HNRT_NOTEBOOK_H


#include <gtkmm.h>
#include "Base/RefObj.h"


namespace hnrt
{
    class Notebook
        : public Gtk::Notebook
        , public RefObj
    {
    public:

        Notebook() {}
        virtual ~Notebook() {}

    private:

        Notebook(const Notebook&);
        void operator =(const Notebook&);
    };
}


#endif //!HNRT_NOTEBOOK_H
