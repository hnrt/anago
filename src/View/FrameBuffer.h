// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_FRAMEBUFFER_H
#define HNRT_FRAMEBUFFER_H


#include <gtkmm.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class FrameBuffer
        : public RefObj
    {
    public:

        static RefPtr<FrameBuffer> create(int cx, int cy, int bpp = 0);

        virtual ~FrameBuffer();
        virtual guchar* getData() const = 0;
        virtual guchar* getData(int x, int y) const = 0;
        virtual int getRowStride() const = 0;
        virtual int getBpp() const = 0;
        virtual void draw(Glib::RefPtr<Gdk::Drawable>& drawable, const Glib::RefPtr<const Gdk::GC>& gc, int x, int y, int cx, int cy, Gdk::RgbDither dith = Gdk::RGB_DITHER_NONE) const = 0;
        virtual void copy(int x, int y, int cx, int cy, const guchar* r, int bpp, int stride) = 0;
        virtual void changeColor(double ratio) = 0;
        int getWidth() const { return _width; }
        int getHeight() const { return _height; }

    protected:

        FrameBuffer(int cx, int cy);
        FrameBuffer(const FrameBuffer&);
        void operator =(const FrameBuffer&);

        int _width;
        int _height;
    };
}


#endif //!HNRT_FRAMEBUFFER_H
