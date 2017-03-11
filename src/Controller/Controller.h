// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONTROLLER_H
#define HNRT_CONTROLLER_H


namespace hnrt
{
    class Controller
    {
    public:

        static void init();
        static void fini();
        static Controller& instance();

        virtual void quit() = 0;
    };
}


#endif //!HNRT_CONTROLLER_H
