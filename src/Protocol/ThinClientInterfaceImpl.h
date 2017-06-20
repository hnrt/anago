// Copyright (C) 2017 Hideaki Narita


// cf. https://github.com/xenserver/xenadmin/CommandLib/thinCLIProtocol.cs


#ifndef HNRT_THINCLIENTINTERFACEIMPL_H
#define HNRT_THINCLIENTINTERFACEIMPL_H


#include <glibmm/ustring.h>
#include "ThinClientInterface.h"


namespace hnrt
{
    class ThinClientInterfaceImpl
        : public ThinClientInterface
    {
    public:

        enum Version
        {
            VERSION_MAJOR = 0,
            VERSION_MINOR = 1,
        };

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

        enum Configuration
        {
            BLOCK_SIZE = 65536,
        };

        ThinClientInterfaceImpl();
        virtual ~ThinClientInterfaceImpl();
        virtual void init();
        virtual void fini();
        virtual void setHostname(const char*);
        virtual void setUsername(const char*);
        virtual void setPassword(const char*);
        virtual void setTimeout(long);
        virtual void setPrintCallback(const sigc::slot<void, ThinClientInterface&>&);
        virtual void setPrintErrorCallback(const sigc::slot<void, ThinClientInterface&>&);
        virtual void setExitCallback(const sigc::slot<void, ThinClientInterface&>&);
        virtual bool run(const char*, ...);
        virtual void cancel() { _cancel = true; }
        virtual const Glib::ustring& getOutput() const { return _output; }
        virtual const Glib::ustring& getErrorOutput() const { return _errorOutput; }
        virtual int getExitCode() const { return _exitCode; }

    protected:

        ThinClientInterfaceImpl(const ThinClientInterfaceImpl&);
        void operator =(const ThinClientInterfaceImpl&);

        Glib::ustring _hostname;
        Glib::ustring _username;
        Glib::ustring _password;
        long _timeoutInMilliseconds;
        bool volatile _cancel;
        Glib::ustring _output;
        Glib::ustring _errorOutput;
        int _exitCode;
        sigc::slot<void, ThinClientInterface&> _printCb;
        sigc::slot<void, ThinClientInterface&> _printErrorCb;
        sigc::slot<void, ThinClientInterface&> _exitCb;
    };
}


#endif //!HNRT_THINCLIENTINTERFACEIMPL_H
