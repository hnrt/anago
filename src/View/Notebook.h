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

        virtual ~Notebook() {}
        virtual const Gtk::Notebook& getInstance() const = 0;
        virtual Gtk::Notebook& getInstance() = 0;
        virtual void update() = 0;
        virtual void updateSnapshots() {}
    };
}


#endif //!HNRT_NOTEBOOK_H
