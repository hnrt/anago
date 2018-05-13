// Copyright (C) 2012-2018 Hideaki Narita


#include <locale.h>
#include <unistd.h>
#include <vector>
#include "LocaleImpl.h"
#include "Process.h"


using namespace hnrt;


LocaleImpl::LocaleImpl()
    : _locale(setlocale(LC_ALL, NULL))
{
}


LocaleImpl::~LocaleImpl()
{
}


//
// Returns the root directory path of the message catalogs.
//
Glib::ustring LocaleImpl::getMessageCatalogDir(const char* textdomainname) const
{
    static const char defaultPath[] = { "./" };

    Glib::ustring firstPart(defaultPath);

    {
        std::vector<Glib::ustring> candidates;

        Glib::ustring lastPart = Glib::ustring::compose("/LC_MESSAGES/%1.mo", textdomainname);
        candidates.push_back(Glib::ustring::compose("%1%2", _locale, lastPart)); // full _locale specification
        ssize_t i = _locale.find('@'); // find @modifier part
        if (i > 0)
        {
            candidates.push_back(Glib::ustring::compose("%1%2", _locale.substr(0, i), lastPart)); // language_territory.codeset
        }
        i = _locale.find('.'); // find .codeset part
        if (i > 0)
        {
            candidates.push_back(Glib::ustring::compose("%1%2", _locale.substr(0, i), lastPart)); // language_territory
        }
        i = _locale.find('_'); // find _territory part
        if (i > 0)
        {
            candidates.push_back(Glib::ustring::compose("%1%2", _locale.substr(0, i), lastPart)); // language
        }

        //
        // directory to check first
        // it is the directory in which the executable is located
        //
        firstPart = Process::instance().getExecutableDirectory();
        if (firstPart.size())
        {
            for (size_t i = 0; i < candidates.size(); i++)
            {
                Glib::ustring filename = Glib::ustring::compose("%1%2", firstPart, candidates[i]);
                if (!access(filename.c_str(), R_OK))
                {
                    goto done;
                }
            }
        }

        //
        // directory to check second
        //
        firstPart = "/usr/local/share/locale/";

        for (size_t i = 0; i < candidates.size(); i++)
        {
            Glib::ustring filename = Glib::ustring::compose("%1%2", firstPart, candidates[i]);
            if (!access(filename.c_str(), R_OK))
            {
                goto done;
            }
        }

        //
        // directory to check last
        //
        firstPart = "/usr/share/locale/";

        for (size_t i = 0; i < candidates.size(); i++)
        {
            Glib::ustring filename = Glib::ustring::compose("%1%2", firstPart, candidates[i]);
            if (!access(filename.c_str(), R_OK))
            {
                goto done;
            }
        }

        //
        // all attempts failed
        //
        firstPart = defaultPath;
    }

done:

    //
    // delete the last separator
    //
    if (firstPart != "/")
    {
        firstPart.resize(firstPart.bytes() - 1);
    }

    return firstPart;
}
