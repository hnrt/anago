// Copyright (C) 2012-2018 Hideaki Narita


#include <string.h>
#include "Base64.h"


using namespace hnrt;


Base64Encoder::Base64Encoder()
{
}


Base64Encoder::Base64Encoder(const void* p, size_t n)
{
    _encode(p, n);
}


Base64Encoder::Base64Encoder(const Base64Encoder& src)
    : ByteString(src)
{
}


Base64Encoder& Base64Encoder::encode(const void* p, size_t n)
{
    _free();
    _value = NULL;
    _encode(p, n);
    return *this;
}


void Base64Encoder::_encode(const void* p, size_t n)
{
    if (!p || !n)
    {
        return;
    }
    static const char table[65] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
    size_t length = ((((n * 8 + 5) / 6) + 3) / 4) * 4;
    _assign(NULL, length, 1);
    const unsigned char* r = reinterpret_cast<const unsigned char*>(p);
    const unsigned char* s = r + n - 3;
    unsigned char* w = _value;
    unsigned int b;
    while (r <= s)
    {
        b = *r++;
        b = (b << 8) | *r++;
        b = (b << 8) | *r++;
        *w++ = table[(b >> (6 * 3)) & 0x3f];
        *w++ = table[(b >> (6 * 2)) & 0x3f];
        *w++ = table[(b >> (6 * 1)) & 0x3f];
        *w++ = table[(b >> (6 * 0)) & 0x3f];
    }
    if (r == s + 1)
    {
        b = *r++;
        b = (b << 8) | *r++;
        b = (b << 8) | 0;
        *w++ = table[(b >> (6 * 3)) & 0x3f];
        *w++ = table[(b >> (6 * 2)) & 0x3f];
        *w++ = table[(b >> (6 * 1)) & 0x3f];
        *w++ = '=';
    }
    else if (r == s + 2)
    {
        b = *r++;
        b = (b << 8) | 0;
        b = (b << 8) | 0;
        *w++ = table[(b >> (6 * 3)) & 0x3f];
        *w++ = table[(b >> (6 * 2)) & 0x3f];
        *w++ = '=';
        *w++ = '=';
    }
    *w = '\0';
}
