// Copyright (C) 2012-2018 Hideaki Narita


#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Scrambler.h"


using namespace hnrt;


static unsigned char salt[48] =
{
    239, // [ 0]
    181, // [ 1]
     31, // [ 2]
     23, // [ 3]
    211, // [ 4]
    251, // [ 5]
    109, // [ 6]
     71, // [ 7]
    163, // [ 8]
     73, // [ 9]
     19, // [10]
     59, // [11]
    137, // [12]
    199, // [13]
    101, // [14]
    103, // [15]
    167, // [16]
    233, // [17]
    127, // [18]
    157, // [19]
     29, // [20]
    227, // [21]
    139, // [22]
    191, // [23]
    107, // [24]
     61, // [25]
    193, // [26]
    149, // [27]
    223, // [28]
    197, // [29]
     83, // [30]
     17, // [31]
    173, // [32]
    241, // [33]
     79, // [34]
     47, // [35]
    113, // [36]
     37, // [37]
    179, // [38]
    131, // [39]
    151, // [40]
     97, // [41]
     43, // [42]
     89, // [43]
    229, // [44]
     53, // [45]
     41, // [46]
     67, // [47]
};


Scrambler::Scrambler(const void *p, size_t n)
    : ScramblerBase(n + 1)
{
    const unsigned char* r = reinterpret_cast<const unsigned char*>(p);
    const unsigned char* s = r + n;
    unsigned char* w = _value;
    *w = static_cast<unsigned char>(time(NULL) & 0xff);
    unsigned char k = salt[*w++ % (sizeof(salt) / sizeof(salt[0]))];
    while (r < s)
    {
        unsigned char c = *r++;
        *w++ = c ^ k;
        k = c;
    }
}


Descrambler::Descrambler(const void *p, size_t n)
    : ScramblerBase(n - 1, 1)
{
    const unsigned char* r = reinterpret_cast<const unsigned char*>(p);
    const unsigned char* s = r + n;
    unsigned char* w = _value;
    unsigned char k = salt[*r++ % (sizeof(salt) / sizeof(salt[0]))];
    while (r < s)
    {
        unsigned char c = *r++;
        k = *w++ = c ^ k;
    }
    *w = '\0';
}
