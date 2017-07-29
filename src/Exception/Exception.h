// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_EXCEPTION_H
#define HNRT_EXCEPTION_H


#include <glibmm/ustring.h>
#include <stdexcept>


namespace hnrt
{
    class Exception
    {
    public:

        Exception();
        Exception(const Exception&);
        Exception(const Glib::ustring&);
        Exception(const char*, ...);
        virtual ~Exception();
        Exception& assign(const Exception&);
        Exception& assignV(const char*, va_list);
        Exception& assign(const char*, ...);
        Exception& operator =(const Exception& other) { return assign(other); }
        const Glib::ustring& what() const { return _what; }

    protected:

        Glib::ustring _what;
    };
}


#endif //!HNRT_EXCEPTION_H
