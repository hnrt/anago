// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_ADDRESSRESOLUTION_H
#define HNRT_ADDRESSRESOLUTION_H


#include <netinet/in.h>
#include <glibmm.h>


namespace hnrt
{
    class AddressResolution
    {
    public:

        struct Record
        {
            unsigned char ip[4];
            int hwType;
            int flags;
            unsigned char mac[6];
            Glib::ustring mask;
            Glib::ustring device;

            Record();
            Record(const Record&);
            Record& assign(const Record& src) { _assign(src); return *this; }
            Record& operator =(const Record& src) { _assign(src); return *this; }
            bool parse(const char*);

        private:

            void _assign(const Record&);
        };

        AddressResolution();
        AddressResolution(const AddressResolution& src);
        ~AddressResolution();
        int getSize() const { return _size; }
        AddressResolution& assign(const AddressResolution& src);
        AddressResolution& operator =(const AddressResolution& src) { return assign(src); }
        const Record& operator [](size_t i) const { return *(_base + i); }
        int getByIpAddress(in_addr_t) const;

    protected:

        void _clear();
        void _extend();
        void _assign(const AddressResolution& src);

        Record* _base;
        int _size;
        int _capacity;
    };
}


#endif //!HNRT_ADDRESSRESOLUTION_H
