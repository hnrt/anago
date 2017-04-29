// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEEXPORTER_H
#define HNRT_VIRTUALMACHINEEXPORTER_H


#include <stdio.h>
#include <time.h>
#include <glibmm/ustring.h>
#include "VirtualMachineOperationState.h"
#include "XenObject.h"


namespace hnrt
{
    class File;
    class VirtualMachine;

    class VirtualMachineExporter
        : public XenObject
    {
    public:

        static RefPtr<VirtualMachineExporter> create(VirtualMachine&);

        ~VirtualMachineExporter();
        void init();
        void fini();
        void run(const char*, bool);
        bool parse(void*, size_t);
        const VirtualMachine& vm() const { return _vm; }
        VirtualMachine& vm() { return _vm; }
        VirtualMachineOperationState state() const { return _state; }
        void abort() { _abort = true; }
        Glib::ustring path();
        int64_t size();
        int64_t nbytes();
        int percent();

    private:

        VirtualMachineExporter(VirtualMachine&);
        VirtualMachineExporter(const VirtualMachineExporter&);
        void operator =(const VirtualMachineExporter&);

        VirtualMachine& _vm;
        RefPtr<File> _xva;
        VirtualMachineOperationState _state;
        int _verified;
        volatile bool _abort;
        time_t _lastUpdated;
    };
}


#endif //!HNRT_VIRTUALMACHINEEXPORTER_H
