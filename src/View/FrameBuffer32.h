// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_FRAMEBUFFER32_H
#define HNRT_FRAMEBUFFER32_H


#include "FrameBuffer.h"


namespace hnrt
{
    class FrameBuffer32
        : public FrameBuffer
    {
    public:

        static RefPtr<FrameBuffer> create(int cx, int cy);

        virtual ~FrameBuffer32();
        virtual guchar* getData() const { return _buffer; }
        virtual guchar* getData(int x, int y) const { return _buffer + (y * _width + x) * 4; }
        virtual int getRowStride() const { return _width * 4; }
        virtual int getBpp() const { return 32; }
        virtual void draw(Glib::RefPtr<Gdk::Drawable>& drawable, const Glib::RefPtr<const Gdk::GC>& gc, int x, int y, int cx, int cy, Gdk::RgbDither dith = Gdk::RGB_DITHER_NONE) const;
        virtual void copy(int x, int y, int cx, int cy, const guchar* r, int bpp, int stride);
        virtual void changeColor(double ratio);

    protected:

        FrameBuffer32(int cx, int cy);
        FrameBuffer32(const FrameBuffer32&);
        void operator =(const FrameBuffer32&);

        guchar* _buffer;
    };
}


#endif //!HNRT_FRAMEBUFFER32_H
