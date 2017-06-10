// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEEXPORTER_H
#define HNRT_VIRTUALMACHINEEXPORTER_H


#include "Protocol/HttpClientHandler.h"
#include "VirtualMachinePorter.h"


namespace hnrt
{
    class VirtualMachineExporter
        : public VirtualMachinePorter
        , public HttpClientHandler
    {
    public:

        static RefPtr<VirtualMachineExporter> create(RefPtr<VirtualMachine>);

        virtual ~VirtualMachineExporter();
        void run(const char*, bool);
        bool getVerify() const { return _verify; }
        void setVerify(bool verify) { _verify = verify; }

        virtual bool onSuccess(HttpClient&, int) { return true; }
        virtual bool onFailure(HttpClient&, const char*) { return false; }
        virtual bool onCancelled(HttpClient&) { return false; }
        virtual size_t read(HttpClient&, void*, size_t) { return 0; }
        virtual bool write(HttpClient&, const void*, size_t);
        virtual void rewind(HttpClient&) {}

    protected:

        VirtualMachineExporter(RefPtr<VirtualMachine>);
        VirtualMachineExporter(const VirtualMachineExporter&);
        void operator =(const VirtualMachineExporter&);
        void init(const char*, bool);

        bool _verify;
    };
}


#endif //!HNRT_VIRTUALMACHINEEXPORTER_H
