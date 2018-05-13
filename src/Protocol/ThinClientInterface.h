// Copyright (C) 2018 Hideaki Narita


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

        typedef sigc::slot<void, const char*> PrintFunction;
        typedef sigc::slot<void, int> ExitFunction;
        typedef sigc::slot<void, size_t> ProgressFunction;

        enum Tag
        {
            PRINT = 0,
            LOAD = 1,
            HTTP_GET = 12,
            HTTP_PUT = 13,
            PROMPT = 3,
            EXIT = 4,
            ERROR = 14,
            OK = 5,
            FAILED = 6,
            CHUNK = 7,
            END = 8,
            COMMAND = 9,
            RESPONSE = 10,
            BLOB = 11,
            DEBUG = 15,
            PRINT_STDERR = 16,
        };

        static RefPtr<ThinClientInterface> create();

        virtual ~ThinClientInterface();
        virtual void init() = 0;
        virtual void fini() = 0;
        virtual void setHostname(const char*) = 0;
        virtual void setUsername(const char*) = 0;
        virtual void setPassword(const char*) = 0;
        virtual void setTimeout(long) = 0;
        virtual void setPrintFunction(const PrintFunction&) = 0;
        virtual void resetPrintFunction() = 0;
        virtual void setPrintErrorFunction(const PrintFunction&) = 0;
        virtual void resetPrintErrorFunction() = 0;
        virtual void setExitFunction(const ExitFunction&) = 0;
        virtual void resetExitFunction() = 0;
        virtual void setProgressFunction(const ProgressFunction&) = 0;
        virtual void resetProgressFunction() = 0;
        virtual bool run(const char*, ...) = 0;
        virtual void cancel() = 0;
        virtual int command() const = 0;
        virtual size_t contentLength() const = 0;

    protected:

        ThinClientInterface();
        ThinClientInterface(const ThinClientInterface&);
        void operator =(const ThinClientInterface&);
    };
}


#endif //!HNRT_THINCLIENTINTERFACE_H
