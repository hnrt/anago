// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_FRAMESCALERIMPL_H
#define HNRT_FRAMESCALERIMPL_H


#include "FrameScaler.h"


namespace hnrt
{
    class FrameScalerImpl
        : public FrameScaler
    {
    public:

        FrameScalerImpl();
        virtual void init();
        virtual void fini();
        virtual void scale(RefPtr<FrameBuffer>, RefPtr<FrameBuffer>, int, int, GdkRectangle&);
        virtual void scaleInParallel(RefPtr<FrameBuffer>, RefPtr<FrameBuffer>, int, int, GdkRectangle&);

    private:

        enum PrivateConstants
        {
            THREAD_COUNT = 4,
        };

        FrameScalerImpl(const FrameScalerImpl&);
        void operator =(const FrameScalerImpl&);
        void run();

        Glib::Mutex _mutexScale;
        Glib::Mutex _mutexStart;
        Glib::Cond _condStart;
        Glib::Mutex _mutexCompleted;
        Glib::Cond _condCompleted;
        Glib::Thread* _threads[THREAD_COUNT];
        GdkRectangle _rects[THREAD_COUNT];
        bool _terminate;
        int _partitionCount;
        int _remaining;
        RefPtr<FrameBuffer> _fb;
        RefPtr<FrameBuffer> _fbScaled;
        int _multiplier;
        int _divisor;
    };
}


#endif //!HNRT_FRAMESCALER_H
