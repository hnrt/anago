// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEPORTER_H
#define HNRT_VIRTUALMACHINEPORTER_H


#include <stdint.h>
#include <time.h>
#include "VirtualMachineOperationState.h"
#include "XenObject.h"


namespace hnrt
{
    class File;

    class VirtualMachinePorter
        : public XenObject
    {
    public:

        virtual ~VirtualMachinePorter();
        RefPtr<VirtualMachine> vm();
        VirtualMachineOperationState state() const { return _state; }
        void abort() { _abort = true; }
        const char* path();
        int64_t size();
        int64_t nbytes();
        int percent();

    protected:

        VirtualMachinePorter(XenObject::Type, Session&, const char*);
        VirtualMachinePorter(const VirtualMachinePorter&);
        void operator =(const VirtualMachinePorter&);
        void init(const char*, const char*, VirtualMachineOperationState::Value);
        void checkVm();

        Glib::ustring _taskId;
        RefPtr<VirtualMachine> _vm;
        RefPtr<File> _xva;
        VirtualMachineOperationState _state;
        volatile bool _abort;
        time_t _lastUpdated;
    };
}


#endif //!HNRT_VIRTUALMACHINEPORTER_H
