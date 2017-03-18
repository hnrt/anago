// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PIXSTORE_H
#define HNRT_PIXSTORE_H


#include <gtkmm.h>
#include "Base/RefPtr.h"


namespace hnrt
{
    class Host;

    class PixStore
    {
    public:

        static void init();
        static void fini();
        static PixStore& instance();

        virtual ~PixStore() {}
        Glib::RefPtr<Gdk::Pixbuf> getApp() const { return _pixYes; }
        Glib::RefPtr<Gdk::Pixbuf> getYes() const { return _pixYes; }
        Glib::RefPtr<Gdk::Pixbuf> getNo() const { return _pixNo; }
        Glib::RefPtr<Gdk::Pixbuf> get(const RefPtr<Host>);

    protected:

        PixStore();
        PixStore(const PixStore&);
        void operator =(const PixStore&);

        Glib::RefPtr<Gdk::Pixbuf> _pixYes;
        Glib::RefPtr<Gdk::Pixbuf> _pixNo;
    };
}


#endif //!HNRT_PIXSTORE_H
