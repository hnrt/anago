// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONSOLE_H
#define HNRT_CONSOLE_H


#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class ConsoleView;

    class Console
        : public RefObj
    {
    public:

        static RefPtr<Console> create(ConsoleView& view);

        virtual ~Console();
        virtual bool isActive() const = 0;
        virtual void open(const char* location, const char* authorization) = 0;
        virtual void close() = 0;
        virtual void run() = 0;
        virtual void terminate() = 0;
        virtual void sendPointerEvent(unsigned char buttonMask, unsigned short x, unsigned short y) = 0;
        virtual void sendKeyEvent(unsigned char downFlag, unsigned int keyval, unsigned int keycode) = 0;
        virtual void sendCtrlAltDelete() = 0;

    protected:

        Console();
        Console(const Console&);
        void operator =(const Console&);
    };
}


#endif //!HNRT_CONSOLE_H
