// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_NOCONTENTSNOTEBOOK_H
#define HNRT_NOCONTENTSNOTEBOOK_H


#include "Notebook.h"


namespace hnrt
{
    class NoContentsNotebook
        : public Notebook
    {
    public:

        static Glib::RefPtr<Notebook> create(const char* = "");

        virtual void update() {}

    protected:

        NoContentsNotebook(const char*);
        NoContentsNotebook(const NoContentsNotebook&);
        void operator =(const NoContentsNotebook&);

        Gtk::VBox _box;
        Gtk::Label _label;
    };
}


#endif //!HNRT_NOCONTENTSNOTEBOOK_H
