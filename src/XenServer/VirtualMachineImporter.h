// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEIMPORTER_H
#define HNRT_VIRTUALMACHINEIMPORTER_H


#include "VirtualMachinePorter.h"


namespace hnrt
{
    class HttpClient;

    class VirtualMachineImporter
        : public VirtualMachinePorter
    {
    public:

        static RefPtr<VirtualMachineImporter> create(Session&);

        virtual ~VirtualMachineImporter();
        void run(const char*);

    protected:

        VirtualMachineImporter(Session&);
        VirtualMachineImporter(const VirtualMachineImporter&);
        void operator =(const VirtualMachineImporter&);
        void init(const char*);
        size_t read(void*, size_t, HttpClient*);
        void rewind();
    };
}


#endif //!HNRT_VIRTUALMACHINEIMPORTER_H
