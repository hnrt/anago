// Copyright (C) 2012-2017 Hideaki Narita


#include <string.h>
#include "Base64.h"


using namespace hnrt;


Base64Decoder::Base64Decoder()
{
}


Base64Decoder::Base64Decoder(const char *p, ssize_t n)
{
    _decode(p, n);
}


Base64Decoder::Base64Decoder(const Base64Decoder& src)
    : ByteString(src)
{
}


Base64Decoder& Base64Decoder::decode(const char *p, ssize_t n)
{
    _free();
    _value = NULL;
    _decode(p, n);
    return *this;
}


void Base64Decoder::_decode(const char* p, ssize_t n)
{
    if (!p)
    {
        return;
    }
    static const unsigned char table[96] =
        {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3e, 0xff, 0xff, 0xff, 0x3f, // 2x
            0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff, 0xff, 0xff, 0x40, 0xff, 0xff, // 3x
            0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, // 4x
            0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff, // 5x
            0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, // 6x
            0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, // 7x
        };
    if (n < 0)
    {
        n = strlen(p);
    }
    const char* r = p;
    const char* s = r + n;
    size_t length = (n * 6 + 7) / 8;
    _assign(NULL, length, 1);
    unsigned char* w = _value;
    unsigned int b = 1, c;
    while (r < s)
    {
        c = (*r++ & 0xff) - 0x20;
        if (c >= 0x60)
        {
            continue;
        }
        c = table[c];
        if (c < 0x40)
        {
            b = (b << 6) | c;
            if ((b & 0x1000000))
            {
                *w++ = static_cast<unsigned char>((b >> (8 * 2)) & 0xff);
                *w++ = static_cast<unsigned char>((b >> (8 * 1)) & 0xff);
                *w++ = static_cast<unsigned char>((b >> (8 * 0)) & 0xff);
                b = 1;
            }
        }
        else if (c == 0x40)
        {
            b <<= 6;
            if ((b & 0x1000000))
            {
                *w++ = static_cast<unsigned char>((b >> (8 * 2)) & 0xff);
                *w++ = static_cast<unsigned char>((b >> (8 * 1)) & 0xff);
            }
            else
            {
                b <<= 6;
                *w++ = static_cast<unsigned char>((b >> (8 * 2)) & 0xff);
            }
            break;
        }
    }
    *w = 0;
    _setLength(w - _value);
}
