// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONSOLEVIEW_H
#define HNRT_CONSOLEVIEW_H


#include <gtkmm.h>
#include <string.h>


namespace hnrt
{
    class ConsoleView
    {
    public:

        virtual void init(int width, int height, int bpp) = 0;
        virtual void resize(int width, int height) = 0;
        virtual void copy(int x, int y, int width, int height, const unsigned char* data, int remaining) = 0;
        virtual void bell() = 0;
        virtual int getDefaultBpp() = 0;

        struct Message
        {
            enum Type
            {
                UNDEFINED,
                RESIZE_DESKTOP,
                UPDATE_DESKTOP,
                BEEP,
            };

            Type type;
            GdkRectangle rect;

            Message()
            {
            }

            Message(Type t)
            {
                type = t;
                rect.x = 0;
                rect.y = 0;
                rect.width = 0;
                rect.height = 0;
            }

            Message(Type t, int x, int y, int cx, int cy)
            {
                type = t;
                rect.x = x;
                rect.y = y;
                rect.width = cx;
                rect.height = cy;
            }

            Message(const Message& src)
            {
                memcpy(this, &src, sizeof(Message));
            }

            void operator =(const Message& src)
            {
                memcpy(this, &src, sizeof(Message));
            }
        };
    };
}


#endif //!HNRT_CONSOLEVIEW_H
