// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEIMPORTER_H
#define HNRT_VIRTUALMACHINEIMPORTER_H


#include "Protocol/HttpClientHandler.h"
#include "VirtualMachinePorter.h"


namespace hnrt
{
    class VirtualMachineImporter
        : public VirtualMachinePorter
        , public HttpClientHandler
    {
    public:

        static RefPtr<VirtualMachineImporter> create(Session&);

        virtual ~VirtualMachineImporter();
        void run(const char*);

        virtual bool onSuccess(HttpClient&, int) { return true; }
        virtual bool onFailure(HttpClient&, const char*) { return false; }
        virtual bool onCancelled(HttpClient&) { return false; }
        virtual size_t read(HttpClient&, void*, size_t);
        virtual bool write(HttpClient&, const void*, size_t) { return false; }
        virtual void rewind(HttpClient&);

    protected:

        VirtualMachineImporter(Session&);
        VirtualMachineImporter(const VirtualMachineImporter&);
        void operator =(const VirtualMachineImporter&);
        void init(const char*);
    };
}


#endif //!HNRT_VIRTUALMACHINEIMPORTER_H
