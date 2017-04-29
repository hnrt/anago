// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEIMPORTER_H
#define HNRT_VIRTUALMACHINEIMPORTER_H


#include "VirtualMachinePorter.h"


namespace hnrt
{
    class VirtualMachineImporter
        : public VirtualMachinePorter
    {
    public:

        static RefPtr<VirtualMachineImporter> create(Session&);

        virtual ~VirtualMachineImporter();
        void run(const char*);
        size_t read(void*, size_t);
        void rewind();

    protected:

        VirtualMachineImporter(Session&);
        VirtualMachineImporter(const VirtualMachineImporter&);
        void operator =(const VirtualMachineImporter&);
        void open(const char*);
        void close();
    };
}


#endif //!HNRT_VIRTUALMACHINEIMPORTER_H
