// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_BASE64_H
#define HNRT_BASE64_H


#include <sys/types.h>
#include "ByteString.h"


namespace hnrt
{
    class Base64Encoder
        : public ByteString
    {
    public:

        Base64Encoder();
        Base64Encoder(const void*, size_t);
        Base64Encoder(const Base64Encoder&);
        Base64Encoder& encode(const void*, size_t);
        const char* getValue() const { return reinterpret_cast<const char*>(_value); }
        operator const char*() const { return getValue(); }

    protected:

        void _encode(const void*, size_t);
    };


    class Base64Decoder
        : public ByteString
    {
    public:

        Base64Decoder();
        Base64Decoder(const char*, ssize_t = -1);
        Base64Decoder(const Base64Decoder&);
        Base64Decoder& decode(const char*p, ssize_t = -1);

    protected:

        void _decode(const char*, ssize_t);
    };
}


#endif //!HNRT_BASE64_H
