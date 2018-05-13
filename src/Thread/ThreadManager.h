// Copyright (C) 2018 Hideaki Narita


#ifndef HNRT_THREADMANAGER_H
#define HNRT_THREADMANAGER_H


#include <glibmm.h>


namespace hnrt
{
    class ThreadManager
    {
    public:

        static void init();
        static void fini();
        static ThreadManager& instance();

        virtual int count() const = 0;
        virtual bool isMain() const = 0;
        virtual const char* getName() const = 0;
        virtual Glib::Thread* create(const sigc::slot<void>&, bool, const char*) = 0;
    };
}


#endif //!HNRT_THREADMANAGER_H
