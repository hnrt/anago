// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_SCRAMBLERBASE_H
#define HNRT_SCRAMBLERBASE_H


#include <stddef.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class ScramblerBase
        : public RefObj
    {
    public:

        virtual ~ScramblerBase();
        void clear();
        const void* getValue() const { return reinterpret_cast<const void*>(_value); }
        size_t getLength() const { return _length; }
        operator const void*() const { return getValue(); }

    protected:

        ScramblerBase(size_t n, size_t extra = 0);
        ScramblerBase();
        ScramblerBase(const ScramblerBase&);
        void operator =(const ScramblerBase&);
        void free();

        unsigned char* _value;
        size_t _length;
    };
}


#endif //!HNRT_SCRAMBLERBASE_H
