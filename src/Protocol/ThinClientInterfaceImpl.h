// Copyright (C) 2017 Hideaki Narita


// cf. https://github.com/xenserver/xenadmin/CommandLib/thinCLIProtocol.cs


#ifndef HNRT_THINCLIENTINTERFACEIMPL_H
#define HNRT_THINCLIENTINTERFACEIMPL_H


#include <glibmm/ustring.h>
#include <map>
#include "ThinClientInterface.h"


namespace hnrt
{
    class HttpClient;
    class Trace;

    class ThinClientInterfaceImpl
        : public ThinClientInterface
    {
    public:

        enum Version
        {
            VERSION_MAJOR = 0,
            VERSION_MINOR = 1,
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
        virtual void setPrintFunction(const PrintFunction&);
        virtual void resetPrintFunction();
        virtual void setPrintErrorFunction(const PrintFunction&);
        virtual void resetPrintErrorFunction();
        virtual void setExitFunction(const ExitFunction&);
        virtual void resetExitFunction();
        virtual void setProgressFunction(const ProgressFunction&);
        virtual void resetProgressFunction();
        virtual bool run(const char*, ...);
        virtual void cancel() { _cancel = true; }
        virtual int command() const { return _command; }
        virtual size_t contentLength() const { return _contentLength; }

    protected:

        typedef bool (ThinClientInterfaceImpl::*CommandFunction)(HttpClient&);
        typedef std::map<int, CommandFunction> CommandFunctionMap;
        typedef std::pair<int, CommandFunction> CommandFunctionMapEntry;

        ThinClientInterfaceImpl(const ThinClientInterfaceImpl&);
        void operator =(const ThinClientInterfaceImpl&);
        bool print(HttpClient&);
        bool printStderr(HttpClient&);
        bool debug(HttpClient&);
        bool exit(HttpClient&);
        bool error(HttpClient&);
        bool prompt(HttpClient&);
        bool load(HttpClient&);
        bool httpPut(HttpClient&);
        bool httpGet(HttpClient&);
        bool connect(HttpClient&, Glib::ustring&, const char*);

        static CommandFunctionMap _commandFunctionMap;

        Glib::ustring _hostname;
        Glib::ustring _username;
        Glib::ustring _password;
        long _timeoutInMilliseconds;
        bool volatile _cancel;
        PrintFunction _printFunction;
        PrintFunction _printErrorFunction;
        ExitFunction _exitFunction;
        ProgressFunction _progressFunction;
        int _command;
        size_t _contentLength;
        char _buf[BLOCK_SIZE];
    };
}


#endif //!HNRT_THINCLIENTINTERFACEIMPL_H
