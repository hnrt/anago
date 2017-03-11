// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONTROLLER_H
#define HNRT_CONTROLLER_H


#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class Controller
    {
    public:

        enum Notification
        {
            XO_CREATED = 0,
            XO_BUSY,
            XO_NAME,
            XO_STATUS,
            XO_SESSION,
            XO_POWER_STATE,
            XO_RECORD,
            XO_SNAPSHOT,
            XO_DESTROYED = 99,
            XO_MIN = XO_CREATED,
            XO_MAX = XO_DESTROYED,
        };

        static void init();
        static void fini();
        static Controller& instance();

        virtual void parseCommandLine(int argc, char *argv[]) = 0;
        virtual void quit() = 0;
        virtual void incBackgroundCount() = 0;
        virtual void decBackgroundCount() = 0;
        virtual void notify(RefPtr<RefObj>, Notification) = 0;
    };
}


#endif //!HNRT_CONTROLLER_H
