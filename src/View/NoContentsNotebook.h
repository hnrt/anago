// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_NOCONTENTSNOTEBOOK_H
#define HNRT_NOCONTENTSNOTEBOOK_H


#include "Base/RefPtr.h"
#include "Notebook.h"


namespace hnrt
{
    class NoContentsNotebook
        : public Notebook
    {
    public:

        static RefPtr<Notebook> create(const Glib::ustring&);

        virtual const Gtk::Notebook& getInstance() const { return *this; }
        virtual Gtk::Notebook& getInstance() { return *this; }
        virtual void update() {}

    protected:

        NoContentsNotebook(const Glib::ustring&);
        NoContentsNotebook(const NoContentsNotebook&);
        void operator =(const NoContentsNotebook&);

        Gtk::VBox _box;
        Gtk::Label _label;
    };
}


#endif //!HNRT_NOCONTENTSNOTEBOOK_H
