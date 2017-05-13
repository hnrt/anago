// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_LOCALE_H
#define HNRT_LOCALE_H


#include <glibmm.h>


namespace hnrt
{
    class Locale
    {
    public:

        static void init();
        static void fini();
        static Locale& instance();

        //
        // Returns the root directory path of the message catalogs.
        //
        virtual Glib::ustring getMessageCatalogDir(const char* textdomainname) const = 0;
    };
}


#endif //!HNRT_LOCALE_H
