// Copyright (C) 2012-2018 Hideaki Narita


#include "Logger/Logger.h"
#include "FrameBuffer24.h"
#include "FrameBuffer32.h"


using namespace hnrt;


RefPtr<FrameBuffer> FrameBuffer::create(int cx, int cy, int bpp)
{
    Gdk::VisualType vt = Gdk::Visual::get_best_type();
    switch (vt)
    {
    case Gdk::VISUAL_DIRECT_COLOR:
    case Gdk::VISUAL_TRUE_COLOR:
    {
        break;
    }
    default:
    {
        static bool first = true;
        if (first)
        {
            first = false;
            Logger::instance().error("%s is not supported. VISUAL_DIRECT_COLOR is assumed.",
                                     vt == Gdk::VISUAL_STATIC_GRAY ? "STATIC_GRAY" :
                                     vt == Gdk::VISUAL_GRAYSCALE ? "GRAYSCALE" :
                                     vt == Gdk::VISUAL_STATIC_COLOR ? "STATIC_COLOR" :
                                     vt == Gdk::VISUAL_PSEUDO_COLOR ? "PSEUDO_COLOR" :
                                     vt == Gdk::VISUAL_TRUE_COLOR ? "TRUE_COLOR" :
                                     vt == Gdk::VISUAL_DIRECT_COLOR ? "DIRECT_COLOR" :
                                     "?");
        }
        break;
    }
    }

    int depth = Gdk::Visual::get_best_depth();
    switch (depth)
    {
    case 24:
    case 32:
        break;
    default:
    {
        static bool first = true;
        if (first)
        {
            first = false;
            Logger::instance().error("%d depth is not supported.", depth);
        }
        depth = 32;
        break;
    }
    }

    if (!bpp)
    {
        bpp = depth;
    }

    RefPtr<FrameBuffer> p;
    switch (bpp)
    {
    case 24:
        p = FrameBuffer24::create(cx, cy);
        break;
    default:
        Logger::instance().error("FrameBuffer::create: %d depth is not supported.", bpp);
        //FALLTHROUGH
    case 32:
        p = FrameBuffer32::create(cx, cy);
        break;
    }
    return p;
}


FrameBuffer::FrameBuffer(int cx, int cy)
    : _width(cx)
    , _height(cy)
{
}


FrameBuffer::FrameBuffer(const FrameBuffer& src)
    : _width(src._width)
    , _height(src._height)
{
}


FrameBuffer::~FrameBuffer()
{
}
