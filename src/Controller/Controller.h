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

        static void init();
        static void fini();
        static Controller& instance();

        virtual void parseCommandLine(int argc, char *argv[]) = 0;
        virtual void quit() = 0;
        virtual void incBackgroundCount() = 0;
        virtual void decBackgroundCount() = 0;
        virtual void notify(const RefPtr<RefObj>&, int) = 0;
        virtual void addHost() = 0;
        virtual void editHost() = 0;
        virtual void removeHost() = 0;
        virtual void connect() = 0;
        virtual void disconnect() = 0;
        virtual void showAbout() = 0;
    };
}


#endif //!HNRT_CONTROLLER_H
