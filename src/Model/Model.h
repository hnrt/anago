// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MODEL_H
#define HNRT_MODEL_H


namespace hnrt
{
    class Model
    {
    public:

        static void init();
        static void fini();
        static Model& instance();
    };
}


#endif //!HNRT_MODEL_H
