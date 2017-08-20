// Copyright (C) 2012-2017 Hideaki Narita


#include "Base/Atomic.h"
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Thread/ThreadManager.h"
#include "FrameBuffer.h"
#include "FrameScalerImpl.h"


using namespace hnrt;


FrameScalerImpl::FrameScalerImpl()
    : _terminate(true)
    , _partitionCount(0)
    , _remaining(0)
{
    memset(_threads, 0, sizeof(_threads));
}


FrameScalerImpl::~FrameScalerImpl()
{
}


void FrameScalerImpl::init()
{
    TRACEFUN(this, "FrameScalerImpl::init");
    Glib::Mutex::Lock lock(_mutexScale);
    if (_terminate)
    {
        _terminate = false;
        for (unsigned int i = 0; i < THREAD_COUNT; i++)
        {
            _threads[i] = ThreadManager::instance().create(sigc::mem_fun(*this, &FrameScalerImpl::run), true, "FrameScaler");
        }
    }
}


void FrameScalerImpl::fini()
{
    TRACEFUN(this, "FrameScalerImpl::fini");
    Glib::Mutex::Lock lock1(_mutexScale);
    {
        Glib::Mutex::Lock lock2(_mutexStart);
        if (_terminate)
        {
            return;
        }
        _terminate = true;
        _condStart.broadcast();
    }
    for (unsigned int i = 0; i < THREAD_COUNT; i++)
    {
        if (_threads[i])
        {
            _threads[i]->join();
        }
    }
    memset(_threads, 0, sizeof(_threads));
}


void FrameScalerImpl::scale(RefPtr<FrameBuffer> fb, RefPtr<FrameBuffer> fbScaled, int multiplier, int divisor, GdkRectangle& rect)
{
    TRACEFUN(this, "FrameScalerImpl::scale(%d,%d,%d,%d)", rect.x, rect.y, rect.width, rect.height);
    int xStart = (rect.x * multiplier) / divisor;
    int xEnd = ((rect.x + rect.width) * multiplier + divisor - 1) / divisor;
    int xEnd2 = xEnd;
    if (xEnd >= fbScaled->getWidth())
    {
        xEnd2 = fbScaled->getWidth();
        xEnd = xEnd2 - 1;
    }
    int yStart = (rect.y * multiplier) / divisor;
    int yEnd = ((rect.y + rect.height) * multiplier + divisor - 1) / divisor;
    int yEnd2 = yEnd;
    if (yEnd >= fbScaled->getHeight())
    {
        yEnd2 = fbScaled->getHeight();
        yEnd = yEnd2 - 1;
    }
    for (int y = yStart; y < yEnd; y++)
    {
        int yBase = y * divisor / multiplier;
        double yDist = 1.0 * y * divisor / multiplier - yBase;
        for (int x = xStart; x < xEnd; x++)
        {
            int xBase = x * divisor / multiplier;
            double xDist = 1.0 * x * divisor / multiplier - xBase;
            guchar* p1 = fb->getData(xBase + 0, yBase + 0);
            guchar* p2 = fb->getData(xBase + 1, yBase + 0);
            guchar* p3 = fb->getData(xBase + 0, yBase + 1);
            guchar* p4 = fb->getData(xBase + 1, yBase + 1);
            guchar* pDest = fbScaled->getData(x, y);
            double r =
                p1[0] * (1.0 - xDist) * (1.0 - yDist) +
                p2[0] * xDist * (1.0 - yDist) +
                p3[0] * (1.0 - xDist) * yDist +
                p4[0] * xDist * yDist;
            double g =
                p1[1] * (1.0 - xDist) * (1.0 - yDist) +
                p2[1] * xDist * (1.0 - yDist) +
                p3[1] * (1.0 - xDist) * yDist +
                p4[1] * xDist * yDist;
            double b =
                p1[2] * (1.0 - xDist) * (1.0 - yDist) +
                p2[2] * xDist * (1.0 - yDist) +
                p3[2] * (1.0 - xDist) * yDist +
                p4[2] * xDist * yDist;
            pDest[0] = static_cast<guchar>(r);
            pDest[1] = static_cast<guchar>(g);
            pDest[2] = static_cast<guchar>(b);
        }
        if (xEnd < xEnd2)
        {
            int xBase = xEnd * divisor / multiplier;
            double xDist = 1.0 * xEnd * divisor / multiplier - xBase;
            guchar* p1 = fb->getData(xBase + 0, yBase + 0);
            guchar* p3 = fb->getData(xBase + 0, yBase + 1);
            guchar* pDest = fbScaled->getData(xEnd, y);
            double r =
                p1[0] * (1.0 - xDist) * (1.0 - yDist) +
                p3[0] * (1.0 - xDist) * yDist;
            double g =
                p1[1] * (1.0 - xDist) * (1.0 - yDist) +
                p3[1] * (1.0 - xDist) * yDist;
            double b =
                p1[2] * (1.0 - xDist) * (1.0 - yDist) +
                p3[2] * (1.0 - xDist) * yDist;
            pDest[0] = static_cast<guchar>(r);
            pDest[1] = static_cast<guchar>(g);
            pDest[2] = static_cast<guchar>(b);
        }
    }
    if (yEnd < yEnd2)
    {
        int yBase = yEnd * divisor / multiplier;
        double yDist = 1.0 * yEnd * divisor / multiplier - yBase;
        for (int x = xStart; x < xEnd; x++)
        {
            int xBase = x * divisor / multiplier;
            double xDist = 1.0 * x * divisor / multiplier - xBase;
            guchar* p1 = fb->getData(xBase + 0, yBase + 0);
            guchar* p2 = fb->getData(xBase + 1, yBase + 0);
            guchar* pDest = fbScaled->getData(x, yEnd);
            double r =
                p1[0] * (1.0 - xDist) * (1.0 - yDist) +
                p2[0] * xDist * (1.0 - yDist);
            double g =
                p1[1] * (1.0 - xDist) * (1.0 - yDist) +
                p2[1] * xDist * (1.0 - yDist);
            double b =
                p1[2] * (1.0 - xDist) * (1.0 - yDist) +
                p2[2] * xDist * (1.0 - yDist);
            pDest[0] = static_cast<guchar>(r);
            pDest[1] = static_cast<guchar>(g);
            pDest[2] = static_cast<guchar>(b);
        }
        if (xEnd < xEnd2)
        {
            int xBase = xEnd * divisor / multiplier;
            double xDist = 1.0 * xEnd * divisor / multiplier - xBase;
            guchar* p1 = fb->getData(xBase + 0, yBase + 0);
            guchar* pDest = fbScaled->getData(xEnd, yEnd);
            double r =
                p1[0] * (1.0 - xDist) * (1.0 - yDist);
            double g =
                p1[1] * (1.0 - xDist) * (1.0 - yDist);
            double b =
                p1[2] * (1.0 - xDist) * (1.0 - yDist);
            pDest[0] = static_cast<guchar>(r);
            pDest[1] = static_cast<guchar>(g);
            pDest[2] = static_cast<guchar>(b);
        }
    }
    rect.x = xStart;
    rect.y = yStart;
    rect.width = xEnd - xStart;
    rect.height = yEnd - yStart;
}


void FrameScalerImpl::scaleInParallel(RefPtr<FrameBuffer> fb, RefPtr<FrameBuffer> fbScaled, int multiplier, int divisor, GdkRectangle& rect)
{
    TRACEFUN(this, "ConsoleViewImpl::scaleInParallel");
    Glib::Mutex::Lock lock(_mutexScale);
    if (_terminate)
    {
        return;
    }
    const int heightMin = 16;
    if (rect.height <= heightMin)
    {
        scale(fb, fbScaled, multiplier, divisor, rect);
    }
    else
    {
        _mutexCompleted.lock();
        _mutexStart.lock();
        _fb = fb;
        _fbScaled = fbScaled;
        _multiplier = multiplier;
        _divisor = divisor;
        int h = rect.height / THREAD_COUNT;
        if (h < heightMin)
        {
            h = heightMin;
        }
        int x = rect.x;
        int y = rect.y;
        int cx = rect.width;
        int cy = rect.height;
        int i;
        for (i = 0; i < THREAD_COUNT - 1 && cy > 0; i++)
        {
            _rects[i].x = x;
            _rects[i].y = y;
            _rects[i].width = cx;
            _rects[i].height = h < cy ? h : cy;
            y += _rects[i].height;
            cy -= _rects[i].height;
        }
        if (cy > 0)
        {
            _rects[i].x = x;
            _rects[i].y = y;
            _rects[i].width = cx;
            _rects[i].height = cy;
            i++;
        }
        _remaining = _partitionCount = i;
        TRACEPUT("Broadcasting...");
        _condStart.broadcast();
        _mutexStart.unlock();
        _condCompleted.wait(_mutexCompleted);
        _mutexCompleted.unlock();
        int xStart = (rect.x * multiplier) / divisor;
        int xEnd = ((rect.x + rect.width) * multiplier + divisor - 1) / divisor;
        if (xEnd >= fbScaled->getWidth())
        {
            xEnd = fbScaled->getWidth() - 1;
        }
        int yStart = (rect.y * multiplier) / divisor;
        int yEnd = ((rect.y + rect.height) * multiplier + divisor - 1) / divisor;
        if (yEnd >= fbScaled->getHeight())
        {
            yEnd = fbScaled->getHeight() - 1;
        }
        rect.x = xStart;
        rect.y = yStart;
        rect.width = xEnd - xStart;
        rect.height = yEnd - yStart;
    }
}


void FrameScalerImpl::run()
{
    TRACEFUN(this, "FrameScalerImpl::run");
    _mutexStart.lock();
    for (;;)
    {
        if (_terminate)
        {
            goto done;
        }
        while (_partitionCount <= 0)
        {
            TRACEPUT("Waiting...");
            _condStart.wait(_mutexStart);
            if (_terminate)
            {
                goto done;
            }
        }
        int index = --_partitionCount;
        if (index < 0)
        {
            continue;
        }
        TRACEPUT("Resumed.");
        _mutexStart.unlock();
        try
        {
            scale(_fb, _fbScaled, _multiplier, _divisor, _rects[index]);
            int ret = InterlockedDecrement(&_remaining);
            TRACEPUT("Done. remaining=%d", ret);
            if (!ret)
            {
                Glib::Mutex::Lock lock(_mutexCompleted);
                _condCompleted.signal();
            }
        }
        catch (...)
        {
        }
        _mutexStart.lock();
    }
done:
    _mutexStart.unlock();
}
