// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEEXPORTER_H
#define HNRT_VIRTUALMACHINEEXPORTER_H


#include "VirtualMachinePorter.h"


namespace hnrt
{
    class HttpClient;

    class VirtualMachineExporter
        : public VirtualMachinePorter
    {
    public:

        static RefPtr<VirtualMachineExporter> create(RefPtr<VirtualMachine>);

        virtual ~VirtualMachineExporter();
        void run(const char*, bool);
        bool getVerify() const { return _verify; }
        void setVerify(bool verify) { _verify = verify; }

    protected:

        VirtualMachineExporter(RefPtr<VirtualMachine>);
        VirtualMachineExporter(const VirtualMachineExporter&);
        void operator =(const VirtualMachineExporter&);
        void init(const char*, bool);
        bool write(const void*, size_t, HttpClient*);

        bool _verify;
    };
}


#endif //!HNRT_VIRTUALMACHINEEXPORTER_H
