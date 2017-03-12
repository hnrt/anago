// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MODELIMPL_H
#define HNRT_MODELIMPL_H


#include <glibmm.h>
#include "Logger/Logger.h"
#include "Model.h"


namespace hnrt
{
    class Json;

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

        virtual int getWidth();
        virtual void setWidth(int);
        virtual int getHeight();
        virtual void setHeight(int);
        virtual int getPanelWidth();
        virtual void setPanelWidth(int);

    private:

        ModelImpl(const ModelImpl&);
        void operator =(const ModelImpl&);
        void loadV1(const Json&);

        Glib::ustring _path;
        Glib::RecMutex _mutex;
        std::list<Session*> _sessions;

        int _width;
        int _height;
        int _panelWidth;
    };
}


#endif //!HNRT_MODELIMPL_H
