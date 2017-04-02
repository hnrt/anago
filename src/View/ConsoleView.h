// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONSOLEVIEW_H
#define HNRT_CONSOLEVIEW_H


namespace hnrt
{
    class ConsoleView
    {
    public:

        virtual void init(int width, int height, int bpp) = 0;
        virtual void resize(int width, int height) = 0;
        virtual void copy(int x, int y, int width, int height, const unsigned char* data) = 0;
        virtual void bell() = 0;
        virtual int getDefaultBpp() = 0;
    };
}


#endif //!HNRT_CONSOLEVIEW_H
