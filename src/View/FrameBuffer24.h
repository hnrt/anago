// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_FRAMEBUFFER24_H
#define HNRT_FRAMEBUFFER24_H


#include "FrameBuffer.h"


namespace hnrt
{
    class FrameBuffer24
        : public FrameBuffer
    {
    public:

        static RefPtr<FrameBuffer> create(int cx, int cy);

        virtual ~FrameBuffer24();
        virtual guchar* getData() const { return _buffer; }
        virtual guchar* getData(int x, int y) const { return _buffer + (y * _width + x) * 3; }
        virtual int getRowStride() const { return _width * 3; }
        virtual int getBpp() const { return 24; }
        virtual void draw(Glib::RefPtr<Gdk::Drawable>& drawable, const Glib::RefPtr<const Gdk::GC>& gc, int x, int y, int cx, int cy, Gdk::RgbDither dith = Gdk::RGB_DITHER_NONE) const;
        virtual void copy(int x, int y, int cx, int cy, const guchar* r, int bpp, int stride);
        virtual void changeColor(double ratio);

    protected:

        FrameBuffer24(int cx, int cy);
        FrameBuffer24(const FrameBuffer24&);
        void operator =(const FrameBuffer24&);

        guchar* _buffer;
    };
}


#endif //!HNRT_FRAMEBUFFER24_H
