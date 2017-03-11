// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_BYTESTRING_H
#define HNRT_BYTESTRING_H


#include <stddef.h>


namespace hnrt
{
    class ByteString
    {
    public:

        ByteString();
        ByteString(const void*, size_t, size_t = 0);
        ByteString(const ByteString&);
        ~ByteString(); // intentionally non-virtual in order not to create vtable
        const unsigned char* getValue() const { return _value; }
        size_t getLength() const;
        size_t getExtraLength() const;
        ByteString& clear();
        ByteString& assign(const void*, size_t, size_t = 0);
        ByteString& assign(const ByteString&);
        int compare(const void*, size_t);
        int compare(const ByteString& rhs) { return compare(rhs._value, rhs.getLength()); }
        operator const unsigned char*() const { return _value; }
        ByteString& operator =(const ByteString& rhs) { return assign(rhs); }
        bool operator ==(const ByteString& rhs) { return compare(rhs._value, rhs.getLength()) == 0; }
        bool operator !=(const ByteString& rhs) { return compare(rhs._value, rhs.getLength()) != 0; }
        bool operator <(const ByteString& rhs) { return compare(rhs._value, rhs.getLength()) < 0; }
        bool operator <=(const ByteString& rhs) { return compare(rhs._value, rhs.getLength()) <= 0; }
        bool operator >(const ByteString& rhs) { return compare(rhs._value, rhs.getLength()) > 0; }
        bool operator >=(const ByteString& rhs) { return compare(rhs._value, rhs.getLength()) >= 0; }

    protected:

        void _free();
        void _assign(const void*, size_t, size_t);
        void _setLength(size_t);

        unsigned char* _value;
    };
}


#endif //!HNRT_BYTESTRING_H
