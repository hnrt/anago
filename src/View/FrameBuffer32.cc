// Copyright (C) 2012-2017 Hideaki Narita


#include <stdint.h>
#include <string.h>
#include <new>
#include "FrameBuffer32.h"


using namespace hnrt;


RefPtr<FrameBuffer> FrameBuffer32::create(int cx, int cy)
{
    return RefPtr<FrameBuffer>(new FrameBuffer32(cx, cy));
}


FrameBuffer32::FrameBuffer32(int cx, int cy)
    : FrameBuffer(cx, cy)
    , _buffer(NULL)
{
    size_t n = cx * cy * 4;
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


FrameBuffer32::~FrameBuffer32()
{
    if (_buffer)
    {
        free(_buffer);
    }
}


void FrameBuffer32::draw(Glib::RefPtr<Gdk::Drawable>& drawable, const Glib::RefPtr<const Gdk::GC>& gc, int x, int y, int cx, int cy, Gdk::RgbDither dith) const
{
    drawable->draw_rgb_32_image(gc, x, y, cx, cy, dith, getData(x, y), getRowStride());
}


static inline void copyFrom24qTo32q8(const guchar* r, int cx, int cy, int rstride, guchar* w, int wstride)
{
    const guchar* t = r + cy * rstride;
    int m = cx / 8;
    int dr = rstride - cx * 3;
    int dw = wstride - cx * 4;
    while (r < t)
    {
        const guchar* s = r + m * 8 * 3;
        while (r < s)
        {
            // process 8 pixels in each loop
            r += 8 * 3;
            w += 8 * 4;
            uint64_t q0 = *reinterpret_cast<const uint64_t*>(r - 8 * 3);
            uint64_t q1 = *reinterpret_cast<const uint64_t*>(r - 8 * 2);
            uint64_t q2 = *reinterpret_cast<const uint64_t*>(r - 8 * 1);
            *reinterpret_cast<uint64_t*>(w - 8 * 4) =
                ((q0 & 0x0000000000FFFFFF) >> (8 * 0)) | 
                ((q0 & 0x0000FFFFFF000000) << (8 * 1)) ;
            *reinterpret_cast<uint64_t*>(w - 8 * 3) =
                ((q0 & 0xFFFF000000000000) >> (8 * 6)) | 
                ((q1 & 0x00000000000000FF) << (8 * 2)) |
                ((q1 & 0x00000000FFFFFF00) << (8 * 3)) ;
            *reinterpret_cast<uint64_t*>(w - 8 * 2) =
                ((q1 & 0x00FFFFFF00000000) >> (8 * 4)) | 
                ((q1 & 0xFF00000000000000) >> (8 * 3)) |
                ((q2 & 0x000000000000FFFF) << (8 * 5)) ;
            *reinterpret_cast<uint64_t*>(w - 8 * 1) =
                ((q2 & 0x000000FFFFFF0000) >> (8 * 2)) | 
                ((q2 & 0xFFFFFF0000000000) >> (8 * 1)) ;
        }
        r += dr;
        w += dw;
    }
}


static inline void copyFrom24qTo32q(const guchar* r, int cx, int cy, int rstride, guchar* w, int wstride)
{
    const guchar* t = r + cy * rstride;
    int m = cx / 8;
    int n = cx % 8;
    int dr = rstride - cx * 3;
    int dw = wstride - cx * 4;
    while (r < t)
    {
        const guchar* s = r + m * 8 * 3;
        while (r < s)
        {
            // process 8 pixels in each loop
            r += 8 * 3;
            w += 8 * 4;
            uint64_t q0 = *reinterpret_cast<const uint64_t*>(r - 8 * 3);
            uint64_t q1 = *reinterpret_cast<const uint64_t*>(r - 8 * 2);
            uint64_t q2 = *reinterpret_cast<const uint64_t*>(r - 8 * 1);
            *reinterpret_cast<uint64_t*>(w - 8 * 4) =
                ((q0 & 0x0000000000FFFFFF) >> (8 * 0)) | 
                ((q0 & 0x0000FFFFFF000000) << (8 * 1)) ;
            *reinterpret_cast<uint64_t*>(w - 8 * 3) =
                ((q0 & 0xFFFF000000000000) >> (8 * 6)) | 
                ((q1 & 0x00000000000000FF) << (8 * 2)) |
                ((q1 & 0x00000000FFFFFF00) << (8 * 3)) ;
            *reinterpret_cast<uint64_t*>(w - 8 * 2) =
                ((q1 & 0x00FFFFFF00000000) >> (8 * 4)) | 
                ((q1 & 0xFF00000000000000) >> (8 * 3)) |
                ((q2 & 0x000000000000FFFF) << (8 * 5)) ;
            *reinterpret_cast<uint64_t*>(w - 8 * 1) =
                ((q2 & 0x000000FFFFFF0000) >> (8 * 2)) | 
                ((q2 & 0xFFFFFF0000000000) >> (8 * 1)) ;
        }
        r += n * 3;
        w += n * 4;
        switch (n)
        {
        case 7:
            *reinterpret_cast<uint64_t*>(w - 4 * 7) =
                (static_cast<uint64_t>(r[- 3 * 7 + 0]) << (8 * 0)) |
                (static_cast<uint64_t>(r[- 3 * 7 + 1]) << (8 * 1)) |
                (static_cast<uint64_t>(r[- 3 * 7 + 2]) << (8 * 2)) |
                (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 4)) |
                (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 5)) |
                (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 6)) ;
            //FALLTHROUGH
        case 5:
            *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 0)) |
                (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 1)) |
                (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 2)) |
                (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 4)) |
                (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 5)) |
                (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 6)) ;
            //FALLTHROUGH
        case 3:
            *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 0)) |
                (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 1)) |
                (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 2)) |
                (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 4)) |
                (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 5)) |
                (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 6)) ;
            //FALLTHROUGH
        case 1:
            *reinterpret_cast<uint32_t*>(w - 4 * 1) =
                (static_cast<uint32_t>(r[- 3 * 1 + 0]) << (8 * 0)) |
                (static_cast<uint32_t>(r[- 3 * 1 + 1]) << (8 * 1)) |
                (static_cast<uint32_t>(r[- 3 * 1 + 2]) << (8 * 2)) ;
            break;
        case 6:
            *reinterpret_cast<uint64_t*>(w - 4 * 6) =
                (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 0)) |
                (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 1)) |
                (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 2)) |
                (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 4)) |
                (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 5)) |
                (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 6)) ;
            //FALLTHROUGH
        case 4:
            *reinterpret_cast<uint64_t*>(w - 4 * 4) =
                (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 0)) |
                (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 1)) |
                (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 2)) |
                (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 4)) |
                (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 5)) |
                (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 6)) ;
            //FALLTHROUGH
        case 2:
            *reinterpret_cast<uint64_t*>(w - 4 * 2) =
                (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 0)) |
                (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 1)) |
                (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 2)) |
                (static_cast<uint64_t>(r[- 3 * 1 + 0]) << (8 * 4)) |
                (static_cast<uint64_t>(r[- 3 * 1 + 1]) << (8 * 5)) |
                (static_cast<uint64_t>(r[- 3 * 1 + 2]) << (8 * 6)) ;
            break;
        default:
            break;
        }
        r += dr;
        w += dw;
    }
}


static inline void copyFrom24qTo32(const guchar* r, int cx, int cy, int rstride, guchar* w, int wstride)
{
    const guchar* t = r + cy * rstride;
    int m = cx / 8;
    int n = cx % 8;
    int dr = rstride - cx * 3;
    int dw = wstride - cx * 4;
    while (r < t)
    {
        const guchar* s = r + m * 8 * 3;
        if (!(reinterpret_cast<uint64_t>(w) & 7))
        {
            while (r < s)
            {
                // process 8 pixels in each loop
                r += 8 * 3;
                w += 8 * 4;
                uint64_t q0 = *reinterpret_cast<const uint64_t*>(r - 8 * 3);
                uint64_t q1 = *reinterpret_cast<const uint64_t*>(r - 8 * 2);
                uint64_t q2 = *reinterpret_cast<const uint64_t*>(r - 8 * 1);
                *reinterpret_cast<uint64_t*>(w - 8 * 4) =
                    ((q0 & 0x0000000000FFFFFF) >> (8 * 0)) | 
                    ((q0 & 0x0000FFFFFF000000) << (8 * 1)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 3) =
                    ((q0 & 0xFFFF000000000000) >> (8 * 6)) | 
                    ((q1 & 0x00000000000000FF) << (8 * 2)) |
                    ((q1 & 0x00000000FFFFFF00) << (8 * 3)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 2) =
                    ((q1 & 0x00FFFFFF00000000) >> (8 * 4)) | 
                    ((q1 & 0xFF00000000000000) >> (8 * 3)) |
                    ((q2 & 0x000000000000FFFF) << (8 * 5)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 1) =
                    ((q2 & 0x000000FFFFFF0000) >> (8 * 2)) | 
                    ((q2 & 0xFFFFFF0000000000) >> (8 * 1)) ;
            }
            r += n * 3;
            w += n * 4;
            switch (n)
            {
            case 7:
                *reinterpret_cast<uint64_t*>(w - 4 * 7) =
                    (static_cast<uint64_t>(r[- 3 * 7 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 7 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 7 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 5:
                *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                    (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 3:
                *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                    (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 1:
                *reinterpret_cast<uint32_t*>(w - 4 * 1) =
                    (static_cast<uint32_t>(r[- 3 * 1 + 0]) << (8 * 0)) |
                    (static_cast<uint32_t>(r[- 3 * 1 + 1]) << (8 * 1)) |
                    (static_cast<uint32_t>(r[- 3 * 1 + 2]) << (8 * 2)) ;
                break;
            case 6:
                *reinterpret_cast<uint64_t*>(w - 4 * 6) =
                    (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 4:
                *reinterpret_cast<uint64_t*>(w - 4 * 4) =
                    (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 2:
                *reinterpret_cast<uint64_t*>(w - 4 * 2) =
                    (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 1 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 1 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 1 + 2]) << (8 * 6)) ;
                break;
            default:
                break;
            }
        }
        else
        {
            if (r < s)
            {
                // first, process 7 pixels
                r += 8 * 3;
                w += 8 * 4;
                uint64_t q0 = *reinterpret_cast<const uint64_t*>(r - 8 * 3);
                uint64_t q1 = *reinterpret_cast<const uint64_t*>(r - 8 * 2);
                uint64_t q2 = *reinterpret_cast<const uint64_t*>(r - 8 * 1);
                *reinterpret_cast<uint32_t*>(w - 4 * 8) = static_cast<uint32_t>
                    ((q0 & 0x0000000000FFFFFF) >> (8 * 0)) ;
                *reinterpret_cast<uint64_t*>(w - 4 * 7) =
                    ((q0 & 0x0000FFFFFF000000) >> (8 * 3)) |
                    ((q0 & 0xFFFF000000000000) >> (8 * 2)) | 
                    ((q1 & 0x00000000000000FF) << (8 * 6)) ;
                *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                    ((q1 & 0x00000000FFFFFF00) >> (8 * 1)) |
                    ((q1 & 0x00FFFFFF00000000) >> (8 * 0)) ;
                *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                    ((q1 & 0xFF00000000000000) >> (8 * 7)) |
                    ((q2 & 0x000000000000FFFF) << (8 * 1)) |
                    ((q2 & 0x000000FFFFFF0000) << (8 * 2)) ;
                uint64_t q3 = q2;
                while (r < s)
                {
                    // then, process 8 pixels in each loop
                    r += 8 * 3;
                    w += 8 * 4;
                    q0 = *reinterpret_cast<const uint64_t*>(r - 8 * 3);
                    q1 = *reinterpret_cast<const uint64_t*>(r - 8 * 2);
                    q2 = *reinterpret_cast<const uint64_t*>(r - 8 * 1);
                    *reinterpret_cast<uint64_t*>(w - 4 * 9) =
                        ((q3 & 0xFFFFFF0000000000) >> (8 * 5)) |
                        ((q0 & 0x0000000000FFFFFF) << (8 * 4)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 7) =
                        ((q0 & 0x0000FFFFFF000000) >> (8 * 3)) |
                        ((q0 & 0xFFFF000000000000) >> (8 * 2)) | 
                        ((q1 & 0x00000000000000FF) << (8 * 6)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                        ((q1 & 0x00000000FFFFFF00) >> (8 * 1)) |
                        ((q1 & 0x00FFFFFF00000000) >> (8 * 0)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                        ((q1 & 0xFF00000000000000) >> (8 * 7)) |
                        ((q2 & 0x000000000000FFFF) << (8 * 1)) |
                        ((q2 & 0x000000FFFFFF0000) << (8 * 2)) ;
                    q3 = q2;
                }
                // finally, process 1 pixel
                *reinterpret_cast<uint32_t*>(w - 4 * 1) = static_cast<uint32_t>
                    ((q3 & 0xFFFFFF0000000000) >> (8 * 5));
            }
            if (n)
            {
                r += n * 3;
                w += n * 4;
                *reinterpret_cast<uint32_t*>(w - 4 * n) =
                    (static_cast<uint32_t>(r[- 3 * n + 0]) << (8 * 0)) |
                    (static_cast<uint32_t>(r[- 3 * n + 1]) << (8 * 1)) |
                    (static_cast<uint32_t>(r[- 3 * n + 2]) << (8 * 2)) ;
                switch (n)
                {
                case 7:
                    *reinterpret_cast<uint64_t*>(w - 4 * 6) =
                        (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 6)) ;
                    //FALLTHROUGH
                case 5:
                    *reinterpret_cast<uint64_t*>(w - 4 * 4) =
                        (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 6)) ;
                    //FALLTHROUGH
                case 3:
                    *reinterpret_cast<uint64_t*>(w - 4 * 2) =
                        (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 1 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 1 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 1 + 2]) << (8 * 6)) ;
                    break;
                case 6:
                    *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                        (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 6)) ;
                    //FALLTHROUGH
                case 4:
                    *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                        (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 6)) ;
                    //FALLTHROUGH
                case 2:
                    *reinterpret_cast<uint32_t*>(w - 4 * 1) =
                        (static_cast<uint32_t>(r[- 3 * 1 + 0]) << (8 * 0)) |
                        (static_cast<uint32_t>(r[- 3 * 1 + 1]) << (8 * 1)) |
                        (static_cast<uint32_t>(r[- 3 * 1 + 2]) << (8 * 2)) ;
                    break;
                default:
                    break;
                }
            }
        }
        r += dr;
        w += dw;
    }
}


static inline void copyFrom24dTo32q8(const guchar* r, int cx, int cy, int rstride, guchar* w, int wstride)
{
    const guchar* t = r + cy * rstride;
    int m = cx / 8;
    int dr = rstride - cx * 3;
    int dw = wstride - cx * 4;
    while (r < t)
    {
        const guchar* s = r + m * 8 * 3;
        if (r < s)
        {
            // first, process 6 pixels
            r += 8 * 3;
            w += 8 * 4;
            uint64_t q0 = *reinterpret_cast<const uint32_t*>(r - 4 * 6);
            uint64_t q1 = *reinterpret_cast<const uint64_t*>(r - 4 * 5);
            uint64_t q2 = *reinterpret_cast<const uint64_t*>(r - 4 * 3);
            *reinterpret_cast<uint64_t*>(w - 8 * 4) =
                ((q0 & 0x0000000000FFFFFF) >> (8 * 0)) |
                ((q0 & 0x00000000FF000000) << (8 * 1)) |
                ((q1 & 0x000000000000FFFF) << (8 * 5)) ;
            *reinterpret_cast<uint64_t*>(w - 8 * 3) =
                ((q1 & 0x000000FFFFFF0000) >> (8 * 2)) |
                ((q1 & 0xFFFFFF0000000000) >> (8 * 1)) ;
            *reinterpret_cast<uint64_t*>(w - 8 * 2) =
                ((q2 & 0x0000000000FFFFFF) >> (8 * 0)) |
                ((q2 & 0x0000FFFFFF000000) << (8 * 1)) ;
            uint64_t q3 = q2;
            while (r < s)
            {
                // then, process 8 pixels in each loop
                r += 8 * 3;
                w += 8 * 4;
                q0 = *reinterpret_cast<const uint64_t*>(r - 4 * 7);
                q1 = *reinterpret_cast<const uint64_t*>(r - 4 * 5);
                q2 = *reinterpret_cast<const uint64_t*>(r - 4 * 3);
                *reinterpret_cast<uint64_t*>(w - 8 * 5) =
                    ((q3 & 0xFFFF000000000000) >> (8 * 6)) |
                    ((q0 & 0x00000000000000FF) << (8 * 2)) |
                    ((q0 & 0x00000000FFFFFF00) << (8 * 3)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 4) =
                    ((q0 & 0x00FFFFFF00000000) >> (8 * 4)) |
                    ((q0 & 0xFF00000000000000) >> (8 * 3)) |
                    ((q1 & 0x000000000000FFFF) << (8 * 5)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 3) =
                    ((q1 & 0x000000FFFFFF0000) >> (8 * 2)) |
                    ((q1 & 0xFFFFFF0000000000) >> (8 * 1)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 2) =
                    ((q2 & 0x0000000000FFFFFF) >> (8 * 0)) |
                    ((q2 & 0x0000FFFFFF000000) << (8 * 1)) ;
                q3 = q2;
            }
            // finally, process 2 pixels
            q0 = *reinterpret_cast<const uint32_t*>(r - 4 * 1);
            *reinterpret_cast<uint64_t*>(w - 8 * 1) =
                ((q3 & 0xFFFF000000000000) >> (8 * 6)) |
                ((q0 & 0x00000000000000FF) << (8 * 2)) |
                ((q0 & 0x00000000FFFFFF00) << (8 * 3)) ;
        }
        r += dr;
        w += dw;
    }
}


static inline void copyFrom24dTo32q(const guchar* r, int cx, int cy, int rstride, guchar* w, int wstride)
{
    const guchar* t = r + cy * rstride;
    int m = cx / 8;
    int n = cx % 8;
    int dr = rstride - cx * 3;
    int dw = wstride - cx * 4;
    while (r < t)
    {
        const guchar* s = r + m * 8 * 3;
        if (r < s)
        {
            // first, process 6 pixels
            r += 8 * 3;
            w += 8 * 4;
            uint64_t q0 = *reinterpret_cast<const uint32_t*>(r - 4 * 6);
            uint64_t q1 = *reinterpret_cast<const uint64_t*>(r - 4 * 5);
            uint64_t q2 = *reinterpret_cast<const uint64_t*>(r - 4 * 3);
            *reinterpret_cast<uint64_t*>(w - 8 * 4) =
                ((q0 & 0x0000000000FFFFFF) >> (8 * 0)) |
                ((q0 & 0x00000000FF000000) << (8 * 1)) |
                ((q1 & 0x000000000000FFFF) << (8 * 5)) ;
            *reinterpret_cast<uint64_t*>(w - 8 * 3) =
                ((q1 & 0x000000FFFFFF0000) >> (8 * 2)) |
                ((q1 & 0xFFFFFF0000000000) >> (8 * 1)) ;
            *reinterpret_cast<uint64_t*>(w - 8 * 2) =
                ((q2 & 0x0000000000FFFFFF) >> (8 * 0)) |
                ((q2 & 0x0000FFFFFF000000) << (8 * 1)) ;
            uint64_t q3 = q2;
            while (r < s)
            {
                // then, process 8 pixels in each loop
                r += 8 * 3;
                w += 8 * 4;
                q0 = *reinterpret_cast<const uint64_t*>(r - 4 * 7);
                q1 = *reinterpret_cast<const uint64_t*>(r - 4 * 5);
                q2 = *reinterpret_cast<const uint64_t*>(r - 4 * 3);
                *reinterpret_cast<uint64_t*>(w - 8 * 5) =
                    ((q3 & 0xFFFF000000000000) >> (8 * 6)) |
                    ((q0 & 0x00000000000000FF) << (8 * 2)) |
                    ((q0 & 0x00000000FFFFFF00) << (8 * 3)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 4) =
                    ((q0 & 0x00FFFFFF00000000) >> (8 * 4)) |
                    ((q0 & 0xFF00000000000000) >> (8 * 3)) |
                    ((q1 & 0x000000000000FFFF) << (8 * 5)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 3) =
                    ((q1 & 0x000000FFFFFF0000) >> (8 * 2)) |
                    ((q1 & 0xFFFFFF0000000000) >> (8 * 1)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 2) =
                    ((q2 & 0x0000000000FFFFFF) >> (8 * 0)) |
                    ((q2 & 0x0000FFFFFF000000) << (8 * 1)) ;
                q3 = q2;
            }
            // finally, process 2 pixels
            q0 = *reinterpret_cast<const uint32_t*>(r - 4 * 1);
            *reinterpret_cast<uint64_t*>(w - 8 * 1) =
                ((q3 & 0xFFFF000000000000) >> (8 * 6)) |
                ((q0 & 0x00000000000000FF) << (8 * 2)) |
                ((q0 & 0x00000000FFFFFF00) << (8 * 3)) ;
        }
        r += n * 3;
        w += n * 4;
        switch (n)
        {
        case 7:
            *reinterpret_cast<uint64_t*>(w - 4 * 7) =
                (static_cast<uint64_t>(r[- 3 * 7 + 0]) << (8 * 0)) |
                (static_cast<uint64_t>(r[- 3 * 7 + 1]) << (8 * 1)) |
                (static_cast<uint64_t>(r[- 3 * 7 + 2]) << (8 * 2)) |
                (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 4)) |
                (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 5)) |
                (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 6)) ;
            //FALLTHROUGH
        case 5:
            *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 0)) |
                (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 1)) |
                (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 2)) |
                (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 4)) |
                (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 5)) |
                (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 6)) ;
            //FALLTHROUGH
        case 3:
            *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 0)) |
                (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 1)) |
                (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 2)) |
                (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 4)) |
                (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 5)) |
                (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 6)) ;
            //FALLTHROUGH
        case 1:
            *reinterpret_cast<uint32_t*>(w - 4 * 1) =
                (static_cast<uint64_t>(r[- 3 * 1 + 0]) << (8 * 0)) |
                (static_cast<uint64_t>(r[- 3 * 1 + 1]) << (8 * 1)) |
                (static_cast<uint64_t>(r[- 3 * 1 + 2]) << (8 * 2)) ;
            break;
        case 6:
            *reinterpret_cast<uint64_t*>(w - 4 * 6) =
                (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 0)) |
                (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 1)) |
                (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 2)) |
                (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 4)) |
                (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 5)) |
                (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 6)) ;
            //FALLTHROUGH
        case 4:
            *reinterpret_cast<uint64_t*>(w - 4 * 4) =
                (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 0)) |
                (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 1)) |
                (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 2)) |
                (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 4)) |
                (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 5)) |
                (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 6)) ;
            //FALLTHROUGH
        case 2:
            *reinterpret_cast<uint64_t*>(w - 4 * 2) =
                (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 0)) |
                (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 1)) |
                (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 2)) |
                (static_cast<uint64_t>(r[- 3 * 1 + 0]) << (8 * 4)) |
                (static_cast<uint64_t>(r[- 3 * 1 + 1]) << (8 * 5)) |
                (static_cast<uint64_t>(r[- 3 * 1 + 2]) << (8 * 6)) ;
            break;
        default:
            break;
        }
        r += dr;
        w += dw;
    }
}


static inline void copyFrom24dTo32(const guchar* r, int cx, int cy, int rstride, guchar* w, int wstride)
{
    const guchar* t = r + cy * rstride;
    int m = cx / 8;
    int n = cx % 8;
    int dr = rstride - cx * 3;
    int dw = wstride - cx * 4;
    while (r < t)
    {
        const guchar* s = r + m * 8 * 3;
        if (!(reinterpret_cast<uint64_t>(w) & 7))
        {
            if (r < s)
            {
                // first, process 6 pixels
                r += 8 * 3;
                w += 8 * 4;
                uint64_t q0 = *reinterpret_cast<const uint32_t*>(r - 4 * 6);
                uint64_t q1 = *reinterpret_cast<const uint64_t*>(r - 4 * 5);
                uint64_t q2 = *reinterpret_cast<const uint64_t*>(r - 4 * 3);
                *reinterpret_cast<uint64_t*>(w - 8 * 4) =
                    ((q0 & 0x0000000000FFFFFF) >> (8 * 0)) |
                    ((q0 & 0x00000000FF000000) << (8 * 1)) |
                    ((q1 & 0x000000000000FFFF) << (8 * 5)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 3) =
                    ((q1 & 0x000000FFFFFF0000) >> (8 * 2)) |
                    ((q1 & 0xFFFFFF0000000000) >> (8 * 1)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 2) =
                    ((q2 & 0x0000000000FFFFFF) >> (8 * 0)) |
                    ((q2 & 0x0000FFFFFF000000) << (8 * 1)) ;
                uint64_t q3 = q2;
                while (r < s)
                {
                    // then, process 8 pixels in each loop
                    r += 8 * 3;
                    w += 8 * 4;
                    q0 = *reinterpret_cast<const uint64_t*>(r - 4 * 7);
                    q1 = *reinterpret_cast<const uint64_t*>(r - 4 * 5);
                    q2 = *reinterpret_cast<const uint64_t*>(r - 4 * 3);
                    *reinterpret_cast<uint64_t*>(w - 8 * 5) =
                        ((q3 & 0xFFFF000000000000) >> (8 * 6)) |
                        ((q0 & 0x00000000000000FF) << (8 * 2)) |
                        ((q0 & 0x00000000FFFFFF00) << (8 * 3)) ;
                    *reinterpret_cast<uint64_t*>(w - 8 * 4) =
                        ((q0 & 0x00FFFFFF00000000) >> (8 * 4)) |
                        ((q0 & 0xFF00000000000000) >> (8 * 3)) |
                        ((q1 & 0x000000000000FFFF) << (8 * 5)) ;
                    *reinterpret_cast<uint64_t*>(w - 8 * 3) =
                        ((q1 & 0x000000FFFFFF0000) >> (8 * 2)) |
                        ((q1 & 0xFFFFFF0000000000) >> (8 * 1)) ;
                    *reinterpret_cast<uint64_t*>(w - 8 * 2) =
                        ((q2 & 0x0000000000FFFFFF) >> (8 * 0)) |
                        ((q2 & 0x0000FFFFFF000000) << (8 * 1)) ;
                    q3 = q2;
                }
                // finally, process 2 pixels
                q0 = *reinterpret_cast<const uint32_t*>(r - 4 * 1);
                *reinterpret_cast<uint64_t*>(w - 8 * 1) =
                    ((q3 & 0xFFFF000000000000) >> (8 * 6)) |
                    ((q0 & 0x00000000000000FF) << (8 * 2)) |
                    ((q0 & 0x00000000FFFFFF00) << (8 * 3)) ;
            }
            r += n * 3;
            w += n * 4;
            switch (n)
            {
            case 7:
                *reinterpret_cast<uint64_t*>(w - 4 * 7) =
                    (static_cast<uint64_t>(r[- 3 * 7 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 7 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 7 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 5:
                *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                    (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 3:
                *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                    (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 1:
                *reinterpret_cast<uint32_t*>(w - 4 * 1) =
                    (static_cast<uint32_t>(r[- 3 * 1 + 0]) << (8 * 0)) |
                    (static_cast<uint32_t>(r[- 3 * 1 + 1]) << (8 * 1)) |
                    (static_cast<uint32_t>(r[- 3 * 1 + 2]) << (8 * 2)) ;
                break;
            case 6:
                *reinterpret_cast<uint64_t*>(w - 4 * 6) =
                    (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 4:
                *reinterpret_cast<uint64_t*>(w - 4 * 4) =
                    (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 2:
                *reinterpret_cast<uint64_t*>(w - 4 * 2) =
                    (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 1 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 1 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 1 + 2]) << (8 * 6)) ;
                break;
            default:
                break;
            }
        }
        else
        {
            if (r < s)
            {
                // first, process 5 pixels
                r += 8 * 3;
                w += 8 * 4;
                uint64_t q0 = *reinterpret_cast<const uint32_t*>(r - 4 * 6);
                uint64_t q1 = *reinterpret_cast<const uint64_t*>(r - 4 * 5);
                uint64_t q2 = *reinterpret_cast<const uint64_t*>(r - 4 * 3);
                *reinterpret_cast<uint32_t*>(w - 4 * 8) = static_cast<uint32_t>
                    ((q0 & 0x0000000000FFFFFF) >> (8 * 0)) ;
                *reinterpret_cast<uint64_t*>(w - 4 * 7) =
                    ((q0 & 0x00000000FF000000) >> (8 * 3)) |
                    ((q1 & 0x000000000000FFFF) << (8 * 1)) |
                    ((q1 & 0x000000FFFFFF0000) << (8 * 2)) ;
                *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                    ((q1 & 0xFFFFFF0000000000) >> (8 * 5)) |
                    ((q2 & 0x0000000000FFFFFF) << (8 * 4)) ;
                uint64_t q3 = q2;
                while (r < s)
                {
                    // then, process 8 pixels in each loop
                    r += 8 * 3;
                    w += 8 * 4;
                    q0 = *reinterpret_cast<const uint64_t*>(r - 4 * 7);
                    q1 = *reinterpret_cast<const uint64_t*>(r - 4 * 5);
                    q2 = *reinterpret_cast<const uint64_t*>(r - 4 * 3);
                    *reinterpret_cast<uint64_t*>(w - 4 * 11) =
                        ((q3 & 0x0000FFFFFF000000) >> (8 * 3)) |
                        ((q3 & 0xFFFF000000000000) >> (8 * 2)) |
                        ((q0 & 0x00000000000000FF) << (8 * 6)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 9) = q3 |
                        ((q0 & 0x00000000FFFFFF00) >> (8 * 1)) |
                        ((q0 & 0x00FFFFFF00000000) >> (8 * 0)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 7) =
                        ((q0 & 0xFF00000000000000) >> (8 * 7)) |
                        ((q1 & 0x000000000000FFFF) << (8 * 1)) |
                        ((q1 & 0x000000FFFFFF0000) << (8 * 2)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                        ((q1 & 0xFFFFFF0000000000) >> (8 * 5)) |
                        ((q2 & 0x0000000000FFFFFF) << (8 * 4)) ;
                    q3 = q2;
                }
                // finally, process 3 pixels
                q0 = *reinterpret_cast<const uint32_t*>(r - 4 * 1);
                *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                    ((q3 & 0x0000FFFFFF000000) >> (8 * 3)) |
                    ((q3 & 0xFFFF000000000000) >> (8 * 2)) |
                    ((q0 & 0x00000000000000FF) << (8 * 6)) ;
                *reinterpret_cast<uint32_t*>(w - 4 * 1) = static_cast<uint32_t>
                    ((q0 & 0x00000000FFFFFF00) >> (8 * 1)) ;
            }
            if (n)
            {
                r += n * 3;
                w += n * 4;
                *reinterpret_cast<uint32_t*>(w - 4 * n) =
                    (static_cast<uint32_t>(r[- 3 * n + 0]) << (8 * 0)) |
                    (static_cast<uint32_t>(r[- 3 * n + 1]) << (8 * 1)) |
                    (static_cast<uint32_t>(r[- 3 * n + 2]) << (8 * 2)) ;
                switch (n)
                {
                case 7:
                    *reinterpret_cast<uint64_t*>(w - 4 * 6) =
                        (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 6)) ;
                    //FALLTHROUGH
                case 5:
                    *reinterpret_cast<uint64_t*>(w - 4 * 4) =
                        (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 6)) ;
                    //FALLTHROUGH
                case 3:
                    *reinterpret_cast<uint64_t*>(w - 4 * 2) =
                        (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 1 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 1 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 1 + 2]) << (8 * 6)) ;
                    break;
                case 6:
                    *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                        (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 6)) ;
                    //FALLTHROUGH
                case 4:
                    *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                        (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 6)) ;
                    //FALLTHROUGH
                case 2:
                    *reinterpret_cast<uint32_t*>(w - 4 * 1) =
                        (static_cast<uint32_t>(r[- 3 * 1 + 0]) << (8 * 0)) |
                        (static_cast<uint32_t>(r[- 3 * 1 + 1]) << (8 * 1)) |
                        (static_cast<uint32_t>(r[- 3 * 1 + 2]) << (8 * 2)) ;
                    break;
                default:
                    break;
                }
            }
        }
        r += dr;
        w += dw;
    }
}


static inline void copyFrom24bTo32(const guchar* r, int cx, int cy, int rstride, guchar* w, int wstride)
{
    const guchar* t = r + cy * rstride;
    int m = cx / 8;
    int n = cx % 8;
    int dr = rstride - cx * 3;
    int dw = wstride - cx * 4;
    while (r < t)
    {
        const guchar* s = r + m * 8 * 3;
        if (!(reinterpret_cast<uint64_t>(w) & 7))
        {
            while (r < s)
            {
                // process 8 pixels in each loop
                r += 8 * 3;
                w += 8 * 4;
                *reinterpret_cast<uint64_t*>(w - 8 * 4) =
                    (static_cast<uint64_t>(r[- 3 * 8 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 8 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 8 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 7 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 7 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 7 + 2]) << (8 * 6)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 3) =
                    (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 6)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 2) =
                    (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 6)) ;
                *reinterpret_cast<uint64_t*>(w - 8 * 1) =
                    (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 1 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 1 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 1 + 2]) << (8 * 6)) ;
            }
            r += n * 3;
            w += n * 4;
            switch (n)
            {
            case 7:
                *reinterpret_cast<uint64_t*>(w - 4 * 7) =
                    (static_cast<uint64_t>(r[- 3 * 7 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 7 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 7 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 5:
                *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                    (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 3:
                *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                    (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 1:
                *reinterpret_cast<uint32_t*>(w - 4 * 1) =
                    (static_cast<uint32_t>(r[- 3 * 1 + 0]) << (8 * 0)) |
                    (static_cast<uint32_t>(r[- 3 * 1 + 1]) << (8 * 1)) |
                    (static_cast<uint32_t>(r[- 3 * 1 + 2]) << (8 * 2)) ;
                break;
            case 6:
                *reinterpret_cast<uint64_t*>(w - 4 * 6) =
                    (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 4:
                *reinterpret_cast<uint64_t*>(w - 4 * 4) =
                    (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 6)) ;
                //FALLTHROUGH
            case 2:
                *reinterpret_cast<uint64_t*>(w - 4 * 2) =
                    (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 1 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 1 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 1 + 2]) << (8 * 6)) ;
                break;
            default:
                break;
            }
        }
        else
        {
            if (r < s)
            {
                // first, process 7 pixels
                r += 8 * 3;
                w += 8 * 4;
                *reinterpret_cast<uint32_t*>(w - 4 * 8) =
                    (static_cast<uint32_t>(r[- 3 * 8 + 0]) << (8 * 0)) |
                    (static_cast<uint32_t>(r[- 3 * 8 + 1]) << (8 * 1)) |
                    (static_cast<uint32_t>(r[- 3 * 8 + 2]) << (8 * 2)) ;
                *reinterpret_cast<uint64_t*>(w - 4 * 7) =
                    (static_cast<uint64_t>(r[- 3 * 7 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 7 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 7 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 6)) ;
                *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                    (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 6)) ;
                *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                    (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 0)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 1)) |
                    (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 2)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 4)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 5)) |
                    (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 6)) ;
                while (r < s)
                {
                    // then, process 8 pixels in each loop
                    r += 8 * 3;
                    w += 8 * 4;
                    *reinterpret_cast<uint64_t*>(w - 4 * 9) =
                        (static_cast<uint64_t>(r[- 3 * 9 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 9 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 9 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 8 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 8 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 8 + 2]) << (8 * 2)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 7) =
                        (static_cast<uint64_t>(r[- 3 * 7 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 7 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 7 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 6)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                        (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 6)) ;
                    *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                        (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 6)) ;
                }
                // finally, process 1 pixel
                *reinterpret_cast<uint32_t*>(w - 4 * 1) =
                    (static_cast<uint32_t>(r[- 3 * 1 + 0]) << (8 * 0)) |
                    (static_cast<uint32_t>(r[- 3 * 1 + 1]) << (8 * 1)) |
                    (static_cast<uint32_t>(r[- 3 * 1 + 2]) << (8 * 2)) ;
            }
            if (n)
            {
                r += n * 3;
                w += n * 4;
                *reinterpret_cast<uint32_t*>(w - 4 * n) =
                    (static_cast<uint32_t>(r[- 3 * n + 0]) << (8 * 0)) |
                    (static_cast<uint32_t>(r[- 3 * n + 1]) << (8 * 1)) |
                    (static_cast<uint32_t>(r[- 3 * n + 2]) << (8 * 2)) ;
                switch (n)
                {
                case 7:
                    *reinterpret_cast<uint64_t*>(w - 4 * 6) =
                        (static_cast<uint64_t>(r[- 3 * 6 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 6 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 6 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 6)) ;
                    //FALLTHROUGH
                case 5:
                    *reinterpret_cast<uint64_t*>(w - 4 * 4) =
                        (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 6)) ;
                    //FALLTHROUGH
                case 3:
                    *reinterpret_cast<uint64_t*>(w - 4 * 2) =
                        (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 1 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 1 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 1 + 2]) << (8 * 6)) ;
                    break;
                case 6:
                    *reinterpret_cast<uint64_t*>(w - 4 * 5) =
                        (static_cast<uint64_t>(r[- 3 * 5 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 5 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 4 + 2]) << (8 * 6)) ;
                    //FALLTHROUGH
                case 4:
                    *reinterpret_cast<uint64_t*>(w - 4 * 3) =
                        (static_cast<uint64_t>(r[- 3 * 3 + 0]) << (8 * 0)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 1]) << (8 * 1)) |
                        (static_cast<uint64_t>(r[- 3 * 3 + 2]) << (8 * 2)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 0]) << (8 * 4)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 1]) << (8 * 5)) |
                        (static_cast<uint64_t>(r[- 3 * 2 + 2]) << (8 * 6)) ;
                    //FALLTHROUGH
                case 2:
                    *reinterpret_cast<uint32_t*>(w - 4 * 1) =
                        (static_cast<uint32_t>(r[- 3 * 1 + 0]) << (8 * 0)) |
                        (static_cast<uint32_t>(r[- 3 * 1 + 1]) << (8 * 1)) |
                        (static_cast<uint32_t>(r[- 3 * 1 + 2]) << (8 * 2)) ;
                    break;
                default:
                    break;
                }
            }
        }
        r += dr;
        w += dw;
    }
}


void FrameBuffer32::copy(int x, int y, int cx, int cy, const guchar* r, int bpp, int stride)
{
    if (_buffer && 0 <= x && 0 < cx && x + cx <= _width && 0 <= y && 0 < cy && y + cy <= _height)
    {
        guchar* w = _buffer + (y * _width + x) * 4;
        int wstride = _width * 4;
        if (bpp == 32)
        {
            if (cx == _width && cx * 4 == stride)
            {
                size_t n = cx * cy * 4;
                memcpy(w, r, n);
            }
            else
            {
                const guchar* t = r + cy * stride;
                size_t n = cx * 4;
                while (r < t)
                {
                    memcpy(w, r, n);
                    r += stride;
                    w += wstride;
                }
            }
        }
        else if (bpp == 24)
        {
            if (!(reinterpret_cast<uint64_t>(r) & 7) && !(stride & 7))
            {
                if (!(reinterpret_cast<uint64_t>(w) & 7) && !(_width & 1))
                {
                    if (!(cx & 7))
                    {
                        copyFrom24qTo32q8(r, cx, cy, stride, w, wstride);
                    }
                    else
                    {
                        copyFrom24qTo32q(r, cx, cy, stride, w, wstride);
                    }
                }
                else
                {
                    copyFrom24qTo32(r, cx, cy, stride, w, wstride);
                }
            }
            else if (!(reinterpret_cast<uint64_t>(r) & 3) && !(stride & 3))
            {
                if (!(reinterpret_cast<uint64_t>(w) & 7) && !(_width & 1))
                {
                    if (!(cx & 7))
                    {
                        copyFrom24dTo32q8(r, cx, cy, stride, w, wstride);
                    }
                    else
                    {
                        copyFrom24dTo32q(r, cx, cy, stride, w, wstride);
                    }
                }
                else
                {
                    copyFrom24dTo32(r, cx, cy, stride, w, wstride);
                }
            }
            else
            {
                copyFrom24bTo32(r, cx, cy, stride, w, wstride);
            }
        }
        else
        {
            g_printerr("Warning: FrameBuffer32::copy(%d,%d,%d,%d,%d) was ignored.\n", x, y, cx, cy, bpp);
        }
    }
    else
    {
        g_printerr("Warning: FrameBuffer32::copy(%d,%d,%d,%d,%d) was ignored.\n", x, y, cx, cy, bpp);
    }
}


void FrameBuffer32::changeColor(double ratio)
{
    guchar* p = _buffer;
    guchar* s = p + _width * _height * 4;
    if (!(_width & 1))
    {
        while (p < s)
        {
            uint64_t q = *reinterpret_cast<uint64_t*>(p);
            int64_t r0 = (q >> (8 * 0)) & 0xFF;
            int64_t g0 = (q >> (8 * 1)) & 0xFF;
            int64_t b0 = (q >> (8 * 2)) & 0xFF;
            int64_t r1 = (q >> (8 * 4)) & 0xFF;
            int64_t g1 = (q >> (8 * 5)) & 0xFF;
            int64_t b1 = (q >> (8 * 6)) & 0xFF;
            r0 = static_cast<int64_t>(r0 * ratio);
            g0 = static_cast<int64_t>(g0 * ratio);
            b0 = static_cast<int64_t>(b0 * ratio);
            r1 = static_cast<int64_t>(r1 * ratio);
            g1 = static_cast<int64_t>(g1 * ratio);
            b1 = static_cast<int64_t>(b1 * ratio);
            if (r0 < 0) r0 = 0; else if (r0 > 255) r0 = 255;
            if (g0 < 0) g0 = 0; else if (g0 > 255) g0 = 255;
            if (b0 < 0) b0 = 0; else if (b0 > 255) b0 = 255;
            if (r1 < 0) r1 = 0; else if (r1 > 255) r1 = 255;
            if (g1 < 0) g1 = 0; else if (g1 > 255) g1 = 255;
            if (b1 < 0) b1 = 0; else if (b1 > 255) b1 = 255;
            q = (r0 << (8 * 0))
              | (g0 << (8 * 1))
              | (b0 << (8 * 2))
              | (r1 << (8 * 4))
              | (g1 << (8 * 5))
              | (b1 << (8 * 6));
            *reinterpret_cast<uint64_t*>(p) = q;
            p += 8;
        }
    }
    else
    {
        while (p < s)
        {
            uint32_t d = *reinterpret_cast<uint32_t*>(p);
            int32_t r = (d >> (8 * 0)) & 0xFF;
            int32_t g = (d >> (8 * 1)) & 0xFF;
            int32_t b = (d >> (8 * 2)) & 0xFF;
            r = static_cast<int32_t>(r * ratio);
            g = static_cast<int32_t>(g * ratio);
            b = static_cast<int32_t>(b * ratio);
            if (r < 0) r = 0; else if (r > 255) r = 255;
            if (g < 0) g = 0; else if (g > 255) g = 255;
            if (b < 0) b = 0; else if (b > 255) b = 255;
            d = (r << (8 * 0))
              | (g << (8 * 1))
              | (b << (8 * 2));
            *reinterpret_cast<uint32_t*>(p) = d;
            p += 4;
        }
    }
}
