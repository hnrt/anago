// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MODEL_H
#define HNRT_MODEL_H


#include <list>
#include "Base/RefPtr.h"


namespace hnrt
{
    class Host;
    class PatchBase;
    class Session;
    class XenObject;
    struct ConnectSpec;

    class Model
    {
    public:

        static void init();
        static void fini();
        static Model& instance();

        virtual void load() = 0;
        virtual void save() = 0;
        virtual void clear() = 0;
        virtual int get(std::list<RefPtr<Host> >&) = 0;
        virtual void add(const ConnectSpec&) = 0;
        virtual void remove(Session&) = 0;
        virtual void removeAllSessions() = 0;

        virtual void deselectAll() = 0;
        virtual void select(const RefPtr<XenObject>&) = 0;
        virtual int getSelected(std::list<RefPtr<Host> >&) = 0;
        virtual RefPtr<Host> getSelectedHost() = 0;

        virtual RefPtr<PatchBase> getPatchBase() = 0;

        virtual const char* getAppDir() const = 0;

        virtual int getWidth() = 0;
        virtual void setWidth(int) = 0;
        virtual int getHeight() = 0;
        virtual void setHeight(int) = 0;
        virtual int getPane1Width() = 0;
        virtual void setPane1Width(int) = 0;
    };
}


#endif //!HNRT_MODEL_H
