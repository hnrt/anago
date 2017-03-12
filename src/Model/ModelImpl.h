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
        virtual void load();
        virtual void save();
        virtual int get(std::list<Session*>&);
        virtual void add(const ConnectSpec&);
        virtual void remove(Session&);
        virtual void removeAllSessions();

    private:

        ModelImpl(const ModelImpl&);
        void operator =(const ModelImpl&);

        Glib::ustring _path;
        Glib::RecMutex _mutex;
        std::list<Session*> _sessions;
    };
}


#endif //!HNRT_MODELIMPL_H
