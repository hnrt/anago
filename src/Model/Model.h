// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MODEL_H
#define HNRT_MODEL_H


#include <list>


namespace hnrt
{
    class Session;
    struct ConnectSpec;

    class Model
    {
    public:

        static void init();
        static void fini();
        static Model& instance();

        virtual void load() = 0;
        virtual void save() = 0;
        virtual int get(std::list<Session*>&) = 0;
        virtual void add(const ConnectSpec&) = 0;
        virtual void remove(Session&) = 0;
        virtual void removeAllSessions() = 0;
    };
}


#endif //!HNRT_MODEL_H
