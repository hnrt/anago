// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONTROLLERIMPL_H
#define HNRT_CONTROLLERIMPL_H


#include <glibmm.h>
#include "Logger/Logger.h"
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

    private:

        ControllerImpl(const ControllerImpl&);
        void operator =(const ControllerImpl&);

        Logger& _log;
    };
}


#endif //!HNRT_CONTROLLERIMPL_H
