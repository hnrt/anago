// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_XVA_H
#define HNRT_XVA_H


#include "File/FileImpl.h"


namespace hnrt
{
    class XenObject;

    class VirtualMachineArchive
        : public FileImpl
    {
    public:

        static RefPtr<File> create(const char*, const char*, XenObject&);

        virtual ~VirtualMachineArchive();
        virtual bool validate(volatile bool&);

    protected:

        VirtualMachineArchive(const char*, const char*, XenObject&);
        VirtualMachineArchive(const VirtualMachineArchive&);
        void operator =(const VirtualMachineArchive&);

        XenObject& _owner;
    };
}


#endif //!HNRT_XVA_H
