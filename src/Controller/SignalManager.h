// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_SIGNALMANAGER_H
#define HNRT_SIGNALMANAGER_H


#include <sigc++/sigc++.h>
#include "Base/RefPtr.h"


namespace hnrt
{
    class XenObject;

    class SignalManager
    {
    public:

        typedef sigc::signal<void, RefPtr<XenObject>, int> XenObjectSignal;

        static void init();
        static void fini();
        static SignalManager& instance();

        virtual void clear() = 0;
        virtual XenObjectSignal xenObjectSignal(int) = 0;
        virtual XenObjectSignal xenObjectSignal(const XenObject&) = 0;
        virtual void notify(const RefPtr<XenObject>&, int) = 0;
    };
}


#endif //!HNRT_SIGNALMANAGER_H
