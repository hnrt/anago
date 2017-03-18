// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MODELIMPL_H
#define HNRT_MODELIMPL_H


#include <glibmm.h>
#include "Logger/Logger.h"
#include "Model.h"


namespace hnrt
{
    class Json;
    class Host;

    class ModelImpl
        : public Model
    {
    public:

        ModelImpl();
        ~ModelImpl();
        void init();
        void fini();
        virtual void load();
        virtual void save();
        virtual void clear();
        virtual int get(std::list<RefPtr<Host> >&);
        virtual void add(const ConnectSpec&);
        virtual void remove(Session&);
        virtual void removeAllSessions();

        virtual void deselectAll();
        virtual void select(const RefPtr<XenObject>&);
        virtual int getSelected(std::list<RefPtr<Host> >&);
        virtual RefPtr<Host> getSelectedHost();

        virtual RefPtr<PatchBase> getPatchBase();

        virtual const char* getAppDir() const { return _appDir.c_str(); }

        virtual int getWidth();
        virtual void setWidth(int);
        virtual int getHeight();
        virtual void setHeight(int);
        virtual int getPane1Width();
        virtual void setPane1Width(int);

    private:

        ModelImpl(const ModelImpl&);
        void operator =(const ModelImpl&);
        void loadV1(const Json&);

        Glib::ustring _path;
        Glib::ustring _appDir;
        Glib::RecMutex _mutex;
        std::list<RefPtr<Host> > _hosts;
        std::list<RefPtr<XenObject> > _selected;
        RefPtr<PatchBase> _patchBase;

        int _width;
        int _height;
        int _pane1Width;
    };
}


#endif //!HNRT_MODELIMPL_H
