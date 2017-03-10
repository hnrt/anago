// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_LOCALEIMPL_H
#define HNRT_LOCALEIMPL_H


#include "Locale.h"


namespace hnrt
{
    class LocaleImpl
        : public Locale
    {
    public:

        LocaleImpl();
        ~LocaleImpl();

        //
        // Returns the root directory path of the message catalogs.
        //
        virtual Glib::ustring getMessageCatalogDir(const char* textdomainname) const;

    private:

        LocaleImpl(const LocaleImpl&);
        void operator =(const LocaleImpl&);

        Glib::ustring _locale;
    };
}


#endif //!HNRT_LOCALEIMPL_H
