// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_NOTEBOOK_H
#define HNRT_NOTEBOOK_H


#include <gtkmm.h>


namespace hnrt
{
    class Notebook
        : public Gtk::Notebook
    {
    public:

        virtual void update() = 0;
        virtual void updateSnapshots() {}
    };
}


#endif //!HNRT_NOTEBOOK_H
