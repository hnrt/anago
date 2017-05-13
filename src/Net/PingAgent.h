// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PINGAGENT_H
#define HNRT_PINGAGENT_H


namespace hnrt
{
    class PingAgent
    {
    public:

        enum State
        {
            UNKNOWN,
            INACTIVE,
            ACTIVE,
        };

        static void init();
        static void fini();
        static PingAgent& instance();

        virtual void open() = 0;
        virtual void close() = 0;
        virtual void clear() = 0;
        virtual void add(const char*) = 0;
        virtual void remove(const char*) = 0;
        virtual State get(const char*) = 0;
    };
}


#endif //!HNRT_PINGAGENT_H
