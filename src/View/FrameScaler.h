// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_FRAMESCALER_H
#define HNRT_FRAMESCALER_H


#include <gtkmm.h>
#include "Base/RefPtr.h"


namespace hnrt
{
    class FrameBuffer;

    class FrameScaler
    {
    public:

        virtual ~FrameScaler() {}
        virtual void init() = 0;
        virtual void fini() = 0;
        virtual void scale(RefPtr<FrameBuffer>, RefPtr<FrameBuffer>, int, int, GdkRectangle&) = 0;
        virtual void scaleInParallel(RefPtr<FrameBuffer>, RefPtr<FrameBuffer>, int, int, GdkRectangle&) = 0;

        typedef void (FrameScaler::*ScaleFunc)(RefPtr<FrameBuffer>, RefPtr<FrameBuffer>, int, int, GdkRectangle&);
    };
}


#endif //!HNRT_FRAMESCALER_H
