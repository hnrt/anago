// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONSOLEVIEWKEYBOARDINPUTFILTER_H
#define HNRT_CONSOLEVIEWKEYBOARDINPUTFILTER_H


#include "Base/RefObj.h"


namespace hnrt
{
    class ConsoleViewKeyboardInputFilter
        : public RefObj
    {
    public:

        virtual void reset() = 0;
        virtual bool filter(unsigned char downFlag, unsigned int& keyval, unsigned int& state, unsigned int& keycode) = 0;
    };
}


#endif //!HNRT_CONSOLEVIEWKEYBOARDINPUTFILTER_H
