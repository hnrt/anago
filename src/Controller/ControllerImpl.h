// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONTROLLERIMPL_H
#define HNRT_CONTROLLERIMPL_H


#include <glibmm.h>
#include "Controller.h"


namespace hnrt
{
    class ControllerImpl
        : public Controller
    {
    public:

        ControllerImpl();
        ~ControllerImpl();
        virtual void parseCommandLine(int argc, char *argv[]);
        virtual void quit();
        virtual void incBackgroundCount();
        virtual void decBackgroundCount();

    private:

        ControllerImpl(const ControllerImpl&);
        void operator =(const ControllerImpl&);
        bool quit1();

        volatile int _backgroundCount;
        bool _quitInProgress;
    };
}


#endif //!HNRT_CONTROLLERIMPL_H
