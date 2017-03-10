// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PROCESS_H
#define HNRT_PROCESS_H


#include <glibmm.h>


namespace hnrt
{
    class Process
    {
    public:

        static void init();
        static void fini();
        static Process& instance();

        virtual Glib::ustring getExecutablePath() const = 0;
        virtual Glib::ustring getExecutableDirectory() const = 0;
    };
}


#endif //!HNRT_PROCESS_H
