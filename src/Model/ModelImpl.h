// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MODELIMPL_H
#define HNRT_MODELIMPL_H


#include <glibmm.h>
#include "Logger/Logger.h"
#include "Model.h"


namespace hnrt
{
    class ModelImpl
        : public Model
    {
    public:

        ModelImpl();
        ~ModelImpl();

    private:

        ModelImpl(const ModelImpl&);
        void operator =(const ModelImpl&);

        Logger& _log;
    };
}


#endif //!HNRT_MODELIMPL_H
