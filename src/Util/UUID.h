// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_UUID_H
#define HNRT_UUID_H


#include <glibmm.h>


namespace hnrt
{
    class UUID
    {
    public:

        static Glib::ustring generate();
        static UUID parse(const char*, char** = 0);

        UUID();
        UUID(const UUID&);
        ~UUID();
        UUID& set();
        UUID& operator =(const UUID&);
        Glib::ustring toString() const;
        bool operator ==(const UUID&) const;
        bool operator !=(const UUID&) const;
        bool isNull() const;

    protected:

        unsigned int data1;
        unsigned short data2;
        unsigned short data3;
        unsigned char data4[8];
    };
}


#endif //!HNRT_UUID_H
