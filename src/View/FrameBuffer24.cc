// Copyright (C) 2012-2017 Hideaki Narita


#include <stdint.h>
#include <string.h>
#include <new>
#include "FrameBuffer24.h"


using namespace hnrt;


RefPtr<FrameBuffer> FrameBuffer24::create(int cx, int cy)
{
    return RefPtr<FrameBuffer>(new FrameBuffer24(cx, cy));
}


FrameBuffer24::FrameBuffer24(int cx, int cy)
    : FrameBuffer(cx, cy)
    , _buffer(NULL)
{
    size_t n = cx * cy * 3;
    if (!n)
    {
        return;
    }

    void* ptr = NULL;
    if (!posix_memalign(&ptr, sizeof(uint64_t), n))
    {
        _buffer = reinterpret_cast<guchar*>(ptr);
        memset(_buffer, 0, n);
    }
    else
    {
        throw std::bad_alloc();
    }
}


FrameBuffer24::~FrameBuffer24()
{
    if (_buffer)
    {
        free(_buffer);
    }
}


void FrameBuffer24::draw(Glib::RefPtr<Gdk::Drawable>& drawable, const Glib::RefPtr<const Gdk::GC>& gc, int x, int y, int cx, int cy, Gdk::RgbDither dith) const
{
    drawable->draw_rgb_image(gc, x, y, cx, cy, dith, getData(x, y), getRowStride());
}


static inline void copyFrom32qTo24(const guchar* r, int cx, int cy, int rstride, guchar* w, int wstride)
{
    const guchar* t = r + cy * rstride;
    int m = cx / 8;
    int n = cx % 8;
    int dr = rstride - cx * 4;
    int dw = wstride - cx * 3;
    while (r < t)
    {
        const guchar* s = r + m * 8 * 4;
        if (!(reinterpret_cast<uint64_t>(w) & 7))
        {
            while (r < s)
            {
                // process 8 pixels in each loop
                r += 8 * 4;
                w += 8 * 3;
                uint64_t q0 = *reinterpret_cast<const uint64_t*>(r - 8 * 4);
                uint64_t q1 = *reinterpret_cast<const uint64_t*>(r - 8 * 3);
                uint64_t q2 = *reinterpret_cast<const uint64_t*>(r - 8 * 2);
                uint64_t q3 = *reinterpret_cast<const uint64_t*>(r - 8 * 1);
                *reinterpret_cast<uint64_t*>(w - 8 * 3) =
                    ((q0 & 0x0000000000FFFFFF) >> (8 * 0)) | 
                    ((q0 & 0x00FFFFFF00000000) >> (8 * 1)) |
                    ((q1 & 0x000000000000FFFF) << (8 * 6)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 2) =
                    ((q1 & 0x0000000000FF0000) >> (8 * 2)) |
                    ((q1 & 0x00FFFFFF00000000) >> (8 * 3)) |
                    ((q2 & 0x0000000000FFFFFF) << (8 * 4)) |
                    ((q2 & 0x000000FF00000000) << (8 * 3)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 1) =
                    ((q2 & 0x00FFFF0000000000) >> (8 * 5)) |
                    ((q3 & 0x0000000000FFFFFF) << (8 * 2)) |
                    ((q3 & 0x00FFFFFF00000000) << (8 * 1)) ;
            }
        }
        else if (!(reinterpret_cast<uint64_t>(w) & 3))
        {
            if (r < s)
            {
                r += 8 * 4;
                w += 8 * 3;
                uint64_t q0 = *reinterpret_cast<const uint64_t*>(r - 8 * 4);
                uint64_t q1 = *reinterpret_cast<const uint64_t*>(r - 8 * 3);
                uint64_t q2 = *reinterpret_cast<const uint64_t*>(r - 8 * 2);
                uint64_t q3 = *reinterpret_cast<const uint64_t*>(r - 8 * 1);
                *reinterpret_cast<uint32_t*>(w - 4 * 6) = static_cast<uint32_t>(
                    ((q0 & 0x0000000000FFFFFF) >> (8 * 0)) |
                    ((q0 & 0x000000FF00000000) >> (8 * 1)));
                *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                    ((q0 & 0x00FFFF0000000000) >> (8 * 5)) |
                    ((q1 & 0x0000000000FFFFFF) << (8 * 2)) |
                    ((q1 & 0x00FFFFFF00000000) << (8 * 1)) ;
                *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                    ((q2 & 0x0000000000FFFFFF) >> (8 * 0)) |
                    ((q2 & 0x00FFFFFF00000000) >> (8 * 1)) |
                    ((q3 & 0x000000000000FFFF) << (8 * 6)) ;
                uint64_t q4 = q3;
                while (r < s)
                {
                    r += 8 * 4;
                    w += 8 * 3;
                    q0 = *reinterpret_cast<const uint64_t*>(r - 8 * 4);
                    q1 = *reinterpret_cast<const uint64_t*>(r - 8 * 3);
                    q2 = *reinterpret_cast<const uint64_t*>(r - 8 * 2);
                    q3 = *reinterpret_cast<const uint64_t*>(r - 8 * 1);
                    *reinterpret_cast<uint64_t*>(w - 4 * 7) =
                        ((q4 & 0x0000000000FF0000) >> (8 * 2)) |
                        ((q4 & 0x00FFFFFF00000000) >> (8 * 3)) |
                        ((q0 & 0x0000000000FFFFFF) << (8 * 4)) |
                        ((q0 & 0x000000FF00000000) << (8 * 3)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                        ((q0 & 0x00FFFF0000000000) >> (8 * 5)) |
                        ((q1 & 0x0000000000FFFFFF) << (8 * 2)) |
                        ((q1 & 0x00FFFFFF00000000) << (8 * 1)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                        ((q2 & 0x0000000000FFFFFF) >> (8 * 0)) |
                        ((q2 & 0x00FFFFFF00000000) >> (8 * 1)) |
                        ((q3 & 0x000000000000FFFF) << (8 * 6)) ;
                    q4 = q3;
                }
                *reinterpret_cast<uint32_t*>(w - 4 * 1) = static_cast<uint32_t>(
                    ((q4 & 0x0000000000FF0000) >> (8 * 2)) |
                    ((q4 & 0x00FFFFFF00000000) >> (8 * 3)));
            }
        }
        else if (!(reinterpret_cast<uint64_t>(w) & 1))
        {
            if (r < s)
            {
                r += 8 * 4;
                w += 8 * 3;
                uint64_t q0 = *reinterpret_cast<const uint64_t*>(r - 8 * 4);
                uint64_t q1 = *reinterpret_cast<const uint64_t*>(r - 8 * 3);
                uint64_t q2 = *reinterpret_cast<const uint64_t*>(r - 8 * 2);
                uint64_t q3 = *reinterpret_cast<const uint64_t*>(r - 8 * 1);
                *reinterpret_cast<uint16_t*>(w - 2 * 12) = static_cast<uint16_t>
                    ((q0 & 0x000000000000FFFF) >> (8 * 0)) ;
                *reinterpret_cast<uint32_t*>(w - 2 * 11) = static_cast<uint32_t>(
                    ((q0 & 0x0000000000FF0000) >> (8 * 2)) |
                    ((q0 & 0x00FFFFFF00000000) >> (8 * 3)));
                *reinterpret_cast<uint32_t*>(w - 2 *  9) = static_cast<uint32_t>(
                    ((q1 & 0x0000000000FFFFFF) >> (8 * 0)) |
                    ((q1 & 0x000000FF00000000) >> (8 * 1)));
                *reinterpret_cast<uint32_t*>(w - 2 *  7) = static_cast<uint32_t>(
                    ((q1 & 0x00FFFF0000000000) >> (8 * 5)) |
                    ((q2 & 0x000000000000FFFF) << (8 * 2)));
                *reinterpret_cast<uint32_t*>(w - 2 *  5) = static_cast<uint32_t>(
                    ((q2 & 0x0000000000FF0000) >> (8 * 2)) |
                    ((q2 & 0x00FFFFFF00000000) >> (8 * 1)));
                *reinterpret_cast<uint32_t*>(w - 2 *  3) = static_cast<uint32_t>(
                    ((q3 & 0x0000000000FFFFFF) >> (8 * 0)) |
                    ((q3 & 0x000000FF00000000) >> (8 * 1)));
                uint64_t q4 = q3;
                while (r < s)
                {
                    r += 8 * 4;
                    w += 8 * 3;
                    q0 = *reinterpret_cast<const uint64_t*>(r - 8 * 4);
                    q1 = *reinterpret_cast<const uint64_t*>(r - 8 * 3);
                    q2 = *reinterpret_cast<const uint64_t*>(r - 8 * 2);
                    q3 = *reinterpret_cast<const uint64_t*>(r - 8 * 1);
                    *reinterpret_cast<uint32_t*>(w - 2 * 13) = static_cast<uint32_t>(
                        ((q4 & 0x00FFFF0000000000) >> (8 * 5)) |
                        ((q0 & 0x000000000000FFFF) << (8 * 2)));
                    *reinterpret_cast<uint32_t*>(w - 2 * 11) = static_cast<uint32_t>(
                        ((q0 & 0x0000000000FF0000) >> (8 * 2)) |
                        ((q0 & 0x00FFFFFF00000000) >> (8 * 3)));
                    *reinterpret_cast<uint32_t*>(w - 2 *  9) = static_cast<uint32_t>(
                        ((q1 & 0x0000000000FFFFFF) >> (8 * 0)) |
                        ((q1 & 0x000000FF00000000) >> (8 * 1)));
                    *reinterpret_cast<uint32_t*>(w - 2 *  7) = static_cast<uint32_t>(
                        ((q1 & 0x00FFFF0000000000) >> (8 * 5)) |
                        ((q2 & 0x000000000000FFFF) << (8 * 2)));
                    *reinterpret_cast<uint32_t*>(w - 2 *  5) = static_cast<uint32_t>(
                        ((q2 & 0x0000000000FF0000) >> (8 * 2)) |
                        ((q2 & 0x00FFFFFF00000000) >> (8 * 1)));
                    *reinterpret_cast<uint32_t*>(w - 2 *  3) = static_cast<uint32_t>(
                        ((q3 & 0x0000000000FFFFFF) >> (8 * 0)) |
                        ((q3 & 0x000000FF00000000) >> (8 * 1)));
                    q4 = q3;
                }
                *reinterpret_cast<uint16_t*>(w - 2 * 1) = static_cast<uint16_t>(
                    ((q4 & 0x00FFFF0000000000) >> (8 * 5)));
            }
        }
        else
        {
            if (r < s)
            {
                r += 8 * 4;
                w += 8 * 3;
                uint64_t q0 = *reinterpret_cast<const uint64_t*>(r - 8 * 4);
                uint64_t q1 = *reinterpret_cast<const uint64_t*>(r - 8 * 3);
                uint64_t q2 = *reinterpret_cast<const uint64_t*>(r - 8 * 2);
                uint64_t q3 = *reinterpret_cast<const uint64_t*>(r - 8 * 1);
                *reinterpret_cast<uint8_t* >(w - 24) = static_cast<uint8_t >(((q0 & 0x00000000000000FF) >> (8 * 0)));
                *reinterpret_cast<uint16_t*>(w - 23) = static_cast<uint16_t>(((q0 & 0x0000000000FFFF00) >> (8 * 1)));
                *reinterpret_cast<uint16_t*>(w - 21) = static_cast<uint16_t>(((q0 & 0x0000FFFF00000000) >> (8 * 4)));
                *reinterpret_cast<uint16_t*>(w - 19) = static_cast<uint16_t>(((q0 & 0x00FF000000000000) >> (8 * 6)) | ((q1 & 0x00000000000000FF) << (8 * 1)));
                *reinterpret_cast<uint16_t*>(w - 17) = static_cast<uint16_t>(((q1 & 0x0000000000FFFF00) >> (8 * 1)));
                *reinterpret_cast<uint16_t*>(w - 15) = static_cast<uint16_t>(((q1 & 0x0000FFFF00000000) >> (8 * 4)));
                *reinterpret_cast<uint16_t*>(w - 13) = static_cast<uint16_t>(((q1 & 0x00FF000000000000) >> (8 * 6)) | ((q2 & 0x00000000000000FF) << (8 * 1)));
                *reinterpret_cast<uint16_t*>(w - 11) = static_cast<uint16_t>(((q2 & 0x0000000000FFFF00) >> (8 * 1)));
                *reinterpret_cast<uint16_t*>(w -  9) = static_cast<uint16_t>(((q2 & 0x0000FFFF00000000) >> (8 * 4)));
                *reinterpret_cast<uint16_t*>(w -  7) = static_cast<uint16_t>(((q2 & 0x00FF000000000000) >> (8 * 6)) | ((q3 & 0x00000000000000FF) << (8 * 1)));
                *reinterpret_cast<uint16_t*>(w -  5) = static_cast<uint16_t>(((q3 & 0x0000000000FFFF00) >> (8 * 1)));
                *reinterpret_cast<uint16_t*>(w -  3) = static_cast<uint16_t>(((q3 & 0x0000FFFF00000000) >> (8 * 4)));
                uint64_t q4 = q3;
                while (r < s)
                {
                    r += 8 * 4;
                    w += 8 * 3;
                    q0 = *reinterpret_cast<const uint64_t*>(r - 8 * 4);
                    q1 = *reinterpret_cast<const uint64_t*>(r - 8 * 3);
                    q2 = *reinterpret_cast<const uint64_t*>(r - 8 * 2);
                    q3 = *reinterpret_cast<const uint64_t*>(r - 8 * 1);
                    *reinterpret_cast<uint16_t*>(w - 25) = static_cast<uint16_t>(((q4 & 0x00FF000000000000) >> (8 * 6)) | ((q0 & 0x00000000000000FF) << (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w - 23) = static_cast<uint16_t>(((q0 & 0x0000000000FFFF00) >> (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w - 21) = static_cast<uint16_t>(((q0 & 0x0000FFFF00000000) >> (8 * 4)));
                    *reinterpret_cast<uint16_t*>(w - 19) = static_cast<uint16_t>(((q0 & 0x00FF000000000000) >> (8 * 6)) | ((q1 & 0x00000000000000FF) << (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w - 17) = static_cast<uint16_t>(((q1 & 0x0000000000FFFF00) >> (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w - 15) = static_cast<uint16_t>(((q1 & 0x0000FFFF00000000) >> (8 * 4)));
                    *reinterpret_cast<uint16_t*>(w - 13) = static_cast<uint16_t>(((q1 & 0x00FF000000000000) >> (8 * 6)) | ((q2 & 0x00000000000000FF) << (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w - 11) = static_cast<uint16_t>(((q2 & 0x0000000000FFFF00) >> (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w -  9) = static_cast<uint16_t>(((q2 & 0x0000FFFF00000000) >> (8 * 4)));
                    *reinterpret_cast<uint16_t*>(w -  7) = static_cast<uint16_t>(((q2 & 0x00FF000000000000) >> (8 * 6)) | ((q3 & 0x00000000000000FF) << (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w -  5) = static_cast<uint16_t>(((q3 & 0x0000000000FFFF00) >> (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w -  3) = static_cast<uint16_t>(((q3 & 0x0000FFFF00000000) >> (8 * 4)));
                    q4 = q3;
                }
                *reinterpret_cast<uint8_t*>(w - 1) = static_cast<uint8_t>(((q4 & 0x00FF000000000000) >> (8 * 6)));
            }
        }
        r += n * 4;
        w += n * 3;
        switch (n)
        {
        case 7:
            w[- 3 * 7 + 0] = r[- 4 * 7 + 0];
            w[- 3 * 7 + 1] = r[- 4 * 7 + 1];
            w[- 3 * 7 + 2] = r[- 4 * 7 + 2];
            //FALLTHROUGH
        case 6:
            w[- 3 * 6 + 0] = r[- 4 * 6 + 0];
            w[- 3 * 6 + 1] = r[- 4 * 6 + 1];
            w[- 3 * 6 + 2] = r[- 4 * 6 + 2];
            //FALLTHROUGH
        case 5:
            w[- 3 * 5 + 0] = r[- 4 * 5 + 0];
            w[- 3 * 5 + 1] = r[- 4 * 5 + 1];
            w[- 3 * 5 + 2] = r[- 4 * 5 + 2];
            //FALLTHROUGH
        case 4:
            w[- 3 * 4 + 0] = r[- 4 * 4 + 0];
            w[- 3 * 4 + 1] = r[- 4 * 4 + 1];
            w[- 3 * 4 + 2] = r[- 4 * 4 + 2];
            //FALLTHROUGH
        case 3:
            w[- 3 * 3 + 0] = r[- 4 * 3 + 0];
            w[- 3 * 3 + 1] = r[- 4 * 3 + 1];
            w[- 3 * 3 + 2] = r[- 4 * 3 + 2];
            //FALLTHROUGH
        case 2:
            w[- 3 * 2 + 0] = r[- 4 * 2 + 0];
            w[- 3 * 2 + 1] = r[- 4 * 2 + 1];
            w[- 3 * 2 + 2] = r[- 4 * 2 + 2];
            //FALLTHROUGH
        case 1:
            w[- 3 * 1 + 0] = r[- 4 * 1 + 0];
            w[- 3 * 1 + 1] = r[- 4 * 1 + 1];
            w[- 3 * 1 + 2] = r[- 4 * 1 + 2];
            break;
        default:
            break;
        }
        r += dr;
        w += dw;
    }
}


static inline void copyFrom32dTo24(const guchar* r, int cx, int cy, int rstride, guchar* w, int wstride)
{
    const guchar* t = r + cy * rstride;
    int m = cx / 8;
    int n = cx % 8;
    int dr = rstride - cx * 4;
    int dw = wstride - cx * 3;
    while (r < t)
    {
        const guchar* s = r + m * 8 * 4;
        if (!(reinterpret_cast<uint64_t>(w) & 7))
        {
            while (r < s)
            {
                // process 8 pixels in each loop
                r += 8 * 4;
                w += 8 * 3;
                uint64_t q0 =
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 8)) << (32 * 0)) |
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 7)) << (32 * 1)) ;
                uint64_t q1 =
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 6)) << (32 * 0)) |
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 5)) << (32 * 1)) ;
                uint64_t q2 =
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 4)) << (32 * 0)) |
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 3)) << (32 * 1)) ;
                uint64_t q3 =
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 2)) << (32 * 0)) |
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 1)) << (32 * 1)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 3) =
                    ((q0 & 0x0000000000FFFFFF) >> (8 * 0)) | 
                    ((q0 & 0x00FFFFFF00000000) >> (8 * 1)) |
                    ((q1 & 0x000000000000FFFF) << (8 * 6)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 2) =
                    ((q1 & 0x0000000000FF0000) >> (8 * 2)) |
                    ((q1 & 0x00FFFFFF00000000) >> (8 * 3)) |
                    ((q2 & 0x0000000000FFFFFF) << (8 * 4)) |
                    ((q2 & 0x000000FF00000000) << (8 * 3)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 1) =
                    ((q2 & 0x00FFFF0000000000) >> (8 * 5)) |
                    ((q3 & 0x0000000000FFFFFF) << (8 * 2)) |
                    ((q3 & 0x00FFFFFF00000000) << (8 * 1)) ;
            }
        }
        else if (!(reinterpret_cast<uint64_t>(w) & 3))
        {
            if (r < s)
            {
                r += 8 * 4;
                w += 8 * 3;
                uint64_t q0 =
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 8)) << (32 * 0)) |
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 7)) << (32 * 1)) ;
                uint64_t q1 =
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 6)) << (32 * 0)) |
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 5)) << (32 * 1)) ;
                uint64_t q2 =
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 4)) << (32 * 0)) |
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 3)) << (32 * 1)) ;
                uint64_t q3 =
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 2)) << (32 * 0)) |
                    (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 1)) << (32 * 1)) ;
                *reinterpret_cast<uint32_t*>(w - 4 * 6) = static_cast<uint32_t>(
                    ((q0 & 0x0000000000FFFFFF) >> (8 * 0)) |
                    ((q0 & 0x000000FF00000000) >> (8 * 1)));
                *reinterpret_cast<uint64_t*>(w - 4 * 5) = (
                    ((q0 & 0x00FFFF0000000000) >> (8 * 5)) |
                    ((q1 & 0x0000000000FFFFFF) << (8 * 2)) |
                    ((q1 & 0x00FFFFFF00000000) << (8 * 1)));
                *reinterpret_cast<uint64_t*>(w - 4 * 3) = (
                    ((q2 & 0x0000000000FFFFFF) >> (8 * 0)) |
                    ((q2 & 0x00FFFFFF00000000) >> (8 * 1)) |
                    ((q3 & 0x000000000000FFFF) << (8 * 6)));
                uint64_t q4 = q3;
                while (r < s)
                {
                    r += 8 * 4;
                    w += 8 * 3;
                    q0 =
                        (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 8)) << (32 * 0)) |
                        (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 7)) << (32 * 1)) ;
                    q1 =
                        (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 6)) << (32 * 0)) |
                        (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 5)) << (32 * 1)) ;
                    q2 =
                        (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 4)) << (32 * 0)) |
                        (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 3)) << (32 * 1)) ;
                    q3 =
                        (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 2)) << (32 * 0)) |
                        (static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(r - 4 * 1)) << (32 * 1)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 7) =
                        ((q4 & 0x0000000000FF0000) >> (8 * 2)) |
                        ((q4 & 0x00FFFFFF00000000) >> (8 * 3)) |
                        ((q0 & 0x0000000000FFFFFF) << (8 * 4)) |
                        ((q0 & 0x000000FF00000000) << (8 * 3)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                        ((q0 & 0x00FFFF0000000000) >> (8 * 5)) |
                        ((q1 & 0x0000000000FFFFFF) << (8 * 2)) |
                        ((q1 & 0x00FFFFFF00000000) << (8 * 1)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                        ((q2 & 0x0000000000FFFFFF) >> (8 * 0)) |
                        ((q2 & 0x00FFFFFF00000000) >> (8 * 1)) |
                        ((q3 & 0x000000000000FFFF) << (8 * 6)) ;
                    q4 = q3;
                }
                *reinterpret_cast<uint32_t*>(w - 4 * 1) = static_cast<uint32_t>(
                    ((q4 & 0x0000000000FF0000) >> (8 * 2)) |
                    ((q4 & 0x00FFFFFF00000000) >> (8 * 3)));
            }
        }
        else if (!(reinterpret_cast<uint64_t>(w) & 1))
        {
            if (r < s)
            {
                r += 8 * 4;
                w += 8 * 3;
                uint32_t d0 = *reinterpret_cast<const uint32_t*>(r - 4 * 8);
                uint32_t d1 = *reinterpret_cast<const uint32_t*>(r - 4 * 7);
                uint32_t d2 = *reinterpret_cast<const uint32_t*>(r - 4 * 6);
                uint32_t d3 = *reinterpret_cast<const uint32_t*>(r - 4 * 5);
                uint32_t d4 = *reinterpret_cast<const uint32_t*>(r - 4 * 4);
                uint32_t d5 = *reinterpret_cast<const uint32_t*>(r - 4 * 3);
                uint32_t d6 = *reinterpret_cast<const uint32_t*>(r - 4 * 2);
                uint32_t d7 = *reinterpret_cast<const uint32_t*>(r - 4 * 1);
                *reinterpret_cast<uint16_t*>(w - 2 * 12) = static_cast<uint16_t>
                    ((d0 & 0x0000FFFF) >> (8 * 0)) ;
                *reinterpret_cast<uint32_t*>(w - 2 * 11) =
                    ((d0 & 0x00FF0000) >> (8 * 2)) |
                    ((d1 & 0x00FFFFFF) << (8 * 1)) ;
                *reinterpret_cast<uint32_t*>(w - 2 *  9) =
                    ((d2 & 0x00FFFFFF) >> (8 * 0)) |
                    ((d3 & 0x000000FF) << (8 * 3)) ;
                *reinterpret_cast<uint32_t*>(w - 2 *  7) =
                    ((d3 & 0x00FFFF00) >> (8 * 1)) |
                    ((d4 & 0x0000FFFF) << (8 * 2)) ;
                *reinterpret_cast<uint32_t*>(w - 2 *  5) =
                    ((d4 & 0x00FF0000) >> (8 * 2)) |
                    ((d5 & 0x00FFFFFF) << (8 * 1)) ;
                *reinterpret_cast<uint32_t*>(w - 2 *  3) =
                    ((d6 & 0x00FFFFFF) >> (8 * 0)) |
                    ((d7 & 0x000000FF) << (8 * 3)) ;
                uint32_t d8 = d7;
                while (r < s)
                {
                    r += 8 * 4;
                    w += 8 * 3;
                    d0 = *reinterpret_cast<const uint32_t*>(r - 4 * 8);
                    d1 = *reinterpret_cast<const uint32_t*>(r - 4 * 7);
                    d2 = *reinterpret_cast<const uint32_t*>(r - 4 * 6);
                    d3 = *reinterpret_cast<const uint32_t*>(r - 4 * 5);
                    d4 = *reinterpret_cast<const uint32_t*>(r - 4 * 4);
                    d5 = *reinterpret_cast<const uint32_t*>(r - 4 * 3);
                    d6 = *reinterpret_cast<const uint32_t*>(r - 4 * 2);
                    d7 = *reinterpret_cast<const uint32_t*>(r - 4 * 1);
                    *reinterpret_cast<uint32_t*>(w - 2 * 13) =
                        ((d8 & 0x00FFFF00) >> (8 * 1)) |
                        ((d0 & 0x0000FFFF) << (8 * 2)) ;
                    *reinterpret_cast<uint32_t*>(w - 2 * 11) =
                        ((d0 & 0x00FF0000) >> (8 * 2)) |
                        ((d1 & 0x00FFFFFF) << (8 * 1)) ;
                    *reinterpret_cast<uint32_t*>(w - 2 *  9) =
                        ((d2 & 0x00FFFFFF) >> (8 * 0)) |
                        ((d3 & 0x000000FF) << (8 * 3)) ;
                    *reinterpret_cast<uint32_t*>(w - 2 *  7) =
                        ((d3 & 0x00FFFF00) >> (8 * 1)) |
                        ((d4 & 0x0000FFFF) << (8 * 2)) ;
                    *reinterpret_cast<uint32_t*>(w - 2 *  5) =
                        ((d4 & 0x00FF0000) >> (8 * 2)) |
                        ((d5 & 0x00FFFFFF) << (8 * 1)) ;
                    *reinterpret_cast<uint32_t*>(w - 2 *  3) =
                        ((d6 & 0x00FFFFFF) >> (8 * 0)) |
                        ((d7 & 0x000000FF) << (8 * 3)) ;
                    d8 = d7;
                }
                *reinterpret_cast<uint16_t*>(w - 2 * 1) = static_cast<uint16_t>((d8 & 0x00FFFF00) >> (8 * 1));
            }
        }
        else
        {
            if (r < s)
            {
                r += 8 * 4;
                w += 8 * 3;
                uint32_t d0 = *reinterpret_cast<const uint32_t*>(r - 4 * 8);
                uint32_t d1 = *reinterpret_cast<const uint32_t*>(r - 4 * 7);
                uint32_t d2 = *reinterpret_cast<const uint32_t*>(r - 4 * 6);
                uint32_t d3 = *reinterpret_cast<const uint32_t*>(r - 4 * 5);
                uint32_t d4 = *reinterpret_cast<const uint32_t*>(r - 4 * 4);
                uint32_t d5 = *reinterpret_cast<const uint32_t*>(r - 4 * 3);
                uint32_t d6 = *reinterpret_cast<const uint32_t*>(r - 4 * 2);
                uint32_t d7 = *reinterpret_cast<const uint32_t*>(r - 4 * 1);
                *reinterpret_cast<uint8_t* >(w - 24) = static_cast<uint8_t >(((d0 & 0x000000FF) >> (8 * 0)));
                *reinterpret_cast<uint16_t*>(w - 23) = static_cast<uint16_t>(((d0 & 0x00FFFF00) >> (8 * 1)));
                *reinterpret_cast<uint16_t*>(w - 21) = static_cast<uint16_t>(((d1 & 0x0000FFFF) >> (8 * 0)));
                *reinterpret_cast<uint16_t*>(w - 19) = static_cast<uint16_t>(((d1 & 0x00FF0000) >> (8 * 2)) | ((d2 & 0x000000FF) << (8 * 1)));
                *reinterpret_cast<uint16_t*>(w - 17) = static_cast<uint16_t>(((d2 & 0x00FFFF00) >> (8 * 1)));
                *reinterpret_cast<uint16_t*>(w - 15) = static_cast<uint16_t>(((d3 & 0x0000FFFF) >> (8 * 0)));
                *reinterpret_cast<uint16_t*>(w - 13) = static_cast<uint16_t>(((d3 & 0x00FF0000) >> (8 * 2)) | ((d4 & 0x000000FF) << (8 * 1)));
                *reinterpret_cast<uint16_t*>(w - 11) = static_cast<uint16_t>(((d4 & 0x00FFFF00) >> (8 * 1)));
                *reinterpret_cast<uint16_t*>(w -  9) = static_cast<uint16_t>(((d5 & 0x0000FFFF) >> (8 * 0)));
                *reinterpret_cast<uint16_t*>(w -  7) = static_cast<uint16_t>(((d5 & 0x00FF0000) >> (8 * 2)) | ((d6 & 0x000000FF) << (8 * 1)));
                *reinterpret_cast<uint16_t*>(w -  5) = static_cast<uint16_t>(((d6 & 0x00FFFF00) >> (8 * 1)));
                *reinterpret_cast<uint16_t*>(w -  3) = static_cast<uint16_t>(((d7 & 0x0000FFFF) >> (8 * 0)));
                uint32_t d8 = d7;
                while (r < s)
                {
                    r += 8 * 4;
                    w += 8 * 3;
                    d0 = *reinterpret_cast<const uint32_t*>(r - 4 * 8);
                    d1 = *reinterpret_cast<const uint32_t*>(r - 4 * 7);
                    d2 = *reinterpret_cast<const uint32_t*>(r - 4 * 6);
                    d3 = *reinterpret_cast<const uint32_t*>(r - 4 * 5);
                    d4 = *reinterpret_cast<const uint32_t*>(r - 4 * 4);
                    d5 = *reinterpret_cast<const uint32_t*>(r - 4 * 3);
                    d6 = *reinterpret_cast<const uint32_t*>(r - 4 * 2);
                    d7 = *reinterpret_cast<const uint32_t*>(r - 4 * 1);
                    *reinterpret_cast<uint16_t*>(w - 25) = static_cast<uint16_t>(((d8 & 0x00FF0000) >> (8 * 2)) | ((d0 & 0x000000FF) << (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w - 23) = static_cast<uint16_t>(((d0 & 0x00FFFF00) >> (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w - 21) = static_cast<uint16_t>(((d1 & 0x0000FFFF) >> (8 * 0)));
                    *reinterpret_cast<uint16_t*>(w - 19) = static_cast<uint16_t>(((d1 & 0x00FF0000) >> (8 * 2)) | ((d2 & 0x000000FF) << (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w - 17) = static_cast<uint16_t>(((d2 & 0x00FFFF00) >> (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w - 15) = static_cast<uint16_t>(((d3 & 0x0000FFFF) >> (8 * 0)));
                    *reinterpret_cast<uint16_t*>(w - 13) = static_cast<uint16_t>(((d3 & 0x00FF0000) >> (8 * 2)) | ((d4 & 0x000000FF) << (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w - 11) = static_cast<uint16_t>(((d4 & 0x00FFFF00) >> (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w -  9) = static_cast<uint16_t>(((d5 & 0x0000FFFF) >> (8 * 0)));
                    *reinterpret_cast<uint16_t*>(w -  7) = static_cast<uint16_t>(((d5 & 0x00FF0000) >> (8 * 2)) | ((d6 & 0x000000FF) << (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w -  5) = static_cast<uint16_t>(((d6 & 0x00FFFF00) >> (8 * 1)));
                    *reinterpret_cast<uint16_t*>(w -  3) = static_cast<uint16_t>(((d7 & 0x0000FFFF) >> (8 * 0)));
                    d8 = d7;
                }
                *reinterpret_cast<uint8_t*>(w - 1) = static_cast<uint8_t>((d8 & 0x00FF0000) >> (8 * 2));
            }
        }
        r += n * 4;
        w += n * 3;
        switch (n)
        {
        case 7:
            w[- 3 * 7 + 0] = r[- 4 * 7 + 0];
            w[- 3 * 7 + 1] = r[- 4 * 7 + 1];
            w[- 3 * 7 + 2] = r[- 4 * 7 + 2];
            //FALLTHROUGH
        case 6:
            w[- 3 * 6 + 0] = r[- 4 * 6 + 0];
            w[- 3 * 6 + 1] = r[- 4 * 6 + 1];
            w[- 3 * 6 + 2] = r[- 4 * 6 + 2];
            //FALLTHROUGH
        case 5:
            w[- 3 * 5 + 0] = r[- 4 * 5 + 0];
            w[- 3 * 5 + 1] = r[- 4 * 5 + 1];
            w[- 3 * 5 + 2] = r[- 4 * 5 + 2];
            //FALLTHROUGH
        case 4:
            w[- 3 * 4 + 0] = r[- 4 * 4 + 0];
            w[- 3 * 4 + 1] = r[- 4 * 4 + 1];
            w[- 3 * 4 + 2] = r[- 4 * 4 + 2];
            //FALLTHROUGH
        case 3:
            w[- 3 * 3 + 0] = r[- 4 * 3 + 0];
            w[- 3 * 3 + 1] = r[- 4 * 3 + 1];
            w[- 3 * 3 + 2] = r[- 4 * 3 + 2];
            //FALLTHROUGH
        case 2:
            w[- 3 * 2 + 0] = r[- 4 * 2 + 0];
            w[- 3 * 2 + 1] = r[- 4 * 2 + 1];
            w[- 3 * 2 + 2] = r[- 4 * 2 + 2];
            //FALLTHROUGH
        case 1:
            w[- 3 * 1 + 0] = r[- 4 * 1 + 0];
            w[- 3 * 1 + 1] = r[- 4 * 1 + 1];
            w[- 3 * 1 + 2] = r[- 4 * 1 + 2];
            break;
        default:
            break;
        }
        r += dr;
        w += dw;
    }
}


void FrameBuffer24::copy(int x, int y, int cx, int cy, const guchar* r, int bpp, int stride)
{
    if (_buffer && 0 <= x && 0 < cx && x + cx <= _width && 0 <= y && 0 < cy && y + cy <= _height)
    {
        guchar* w = _buffer + (y * _width + x) * 3;
        int wstride = _width * 3;
        if (bpp == 24)
        {
            if (cx == _width && cx * 3 == stride)
            {
                size_t n = cx * cy * 3;
                memcpy(w, r, n);
            }
            else
            {
                const guchar* t = r + cy * stride;
                size_t n = cx * 3;
                while (r < t)
                {
                    memcpy(w, r, n);
                    r += stride;
                    w += wstride;
                }
            }
        }
        else if (bpp == 32)
        {
            if (!(reinterpret_cast<uint64_t>(r) & 7) && !(stride & 7))
            {
                copyFrom32qTo24(r, cx, cy, stride, w, wstride);
            }
            else
            {
                copyFrom32dTo24(r, cx, cy, stride, w, wstride);
            }
        }
        else
        {
            g_printerr("Warning: frame buffer ignored set(%d,%d,%d,%d,%d).\n", x, y, cx, cy, bpp);
        }
    }
    else
    {
        g_printerr("Warning: frame buffer ignored set(%d,%d,%d,%d,%d).\n", x, y, cx, cy, bpp);
    }
}


void FrameBuffer24::changeColor(double ratio)
{
    guchar* p = _buffer;
    guchar* s = p + _width * _height * 3;
    while (p < s)
    {
        int32_t r = static_cast<int32_t>(p[0] * ratio);
        int32_t g = static_cast<int32_t>(p[1] * ratio);
        int32_t b = static_cast<int32_t>(p[2] * ratio);
        if (r < 0) r = 0; else if (r > 255) r = 255;
        if (g < 0) g = 0; else if (g > 255) g = 255;
        if (b < 0) b = 0; else if (b > 255) b = 255;
        p[0] = static_cast<guchar>(r);
        p[1] = static_cast<guchar>(g);
        p[2] = static_cast<guchar>(b);
        p += 3;
    }
}
