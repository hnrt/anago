// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_THINCLIENTINTERFACE_H
#define HNRT_THINCLIENTINTERFACE_H


#include <stddef.h>
#include <sigc++/sigc++.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class ThinClientInterface
        : public RefObj
    {
    public:

        static RefPtr<ThinClientInterface> create();

        virtual ~ThinClientInterface();
        virtual void init() = 0;
        virtual void fini() = 0;
        virtual void setHostname(const char*) = 0;
        virtual void setUsername(const char*) = 0;
        virtual void setPassword(const char*) = 0;
        virtual void setTimeout(long) = 0;
        virtual void setPrintCallback(const sigc::slot<void, ThinClientInterface&>&) = 0;
        virtual void setPrintErrorCallback(const sigc::slot<void, ThinClientInterface&>&) = 0;
        virtual void setExitCallback(const sigc::slot<void, ThinClientInterface&>&) = 0;
        virtual bool run(const char*, ...) = 0;
        virtual void cancel() = 0;
        virtual const Glib::ustring& getOutput() const = 0;
        virtual const Glib::ustring& getErrorOutput() const = 0;
        virtual int getExitCode() const = 0;

    protected:

        ThinClientInterface();
        ThinClientInterface(const ThinClientInterface&);
        void operator =(const ThinClientInterface&);
    };
}


#endif //!HNRT_THINCLIENTINTERFACE_H
