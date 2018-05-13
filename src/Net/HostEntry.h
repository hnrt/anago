// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_HOSTENTRY_H
#define HNRT_HOSTENTRY_H


#include <netdb.h>
#include <netinet/in.h>


namespace hnrt
{
    struct HostEntry
    {
        struct Iter
        {
            Iter(char** pp_ = 0);
            Iter(const Iter&);
            ~Iter();
            const in_addr_t& operator *() const;
            Iter& operator =(const Iter&);
            bool operator ==(const Iter&) const;
            bool operator !=(const Iter&) const;
            Iter& operator ++();
            Iter& operator ++(int);

        private:

            char** pp;
        };

        struct hostent data;
        char* buf;

        HostEntry(const char* name);
        ~HostEntry();
        bool isValid() const;
        operator bool() const { return isValid(); }
        Iter begin() const;
        Iter end() const;

    private:

        HostEntry(const HostEntry&);
        void operator =(const HostEntry&);
    };
}


#endif //!HNRT_HOSTENTRY_H
