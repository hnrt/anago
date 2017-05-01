// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEVERIFIER_H
#define HNRT_VIRTUALMACHINEVERIFIER_H


#include "VirtualMachinePorter.h"


namespace hnrt
{
    class VirtualMachineVerifier
        : public VirtualMachinePorter
    {
    public:

        static RefPtr<VirtualMachineVerifier> create();

        virtual ~VirtualMachineVerifier();
        void run(const char*);

    private:

        VirtualMachineVerifier();
        VirtualMachineVerifier(const VirtualMachineVerifier&);
        void operator =(const VirtualMachineVerifier&);
        void init(const char*);
    };
}


#endif //!HNRT_VIRTUALMACHINEVERIFIER_H
