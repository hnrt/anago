// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_SCRAMBLER_H
#define HNRT_SCRAMBLER_H


#include "ScramblerBase.h"


namespace hnrt
{
    class Scrambler
        : public ScramblerBase
    {
    public:

        Scrambler(const void *p, size_t n);

    private:

        Scrambler(const Scrambler&);
        void operator =(const Scrambler&);
    };


    class Descrambler
        : public ScramblerBase
    {
    public:

        Descrambler(const void *p, size_t n);

    private:

        Descrambler(const Descrambler&);
        void operator =(const Descrambler&);
    };
}


#endif //!HNRT_SCRAMBLE_H
