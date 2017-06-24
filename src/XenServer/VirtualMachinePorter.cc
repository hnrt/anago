// Copyright (C) 2012-2017 Hideaki Narita


#define NO_TRACE


#include "Logger/Trace.h"
#include "Util/UUID.h"
#include "Session.h"
#include "VirtualMachine.h"
#include "VirtualMachineArchive.h"
#include "VirtualMachinePorter.h"
#include "XenObjectStore.h"


using namespace hnrt;


VirtualMachinePorter::VirtualMachinePorter(XenObject::Type type, Session& session, const char* name)
    : XenObject(type, session, NULL, UUID::generate().c_str(), name)
{
    TRACEFUN(this, "VirtualMachinePorter::ctor");
}


VirtualMachinePorter::~VirtualMachinePorter()
{
    TRACEFUN(this, "VirtualMachinePorter::dtor");
    _xva.reset();
    if (_vm)
    {
        _vm->setBusy(false);
        _vm.reset();
    }
}


// Note: Lock needs to be held.
void VirtualMachinePorter::init(const char* path, const char* mode, VirtualMachineOperationState::Value initState)
{
    _xva = VirtualMachineArchive::create(path, mode, *this);
    _state = initState;
    _abort = false;
    _lastUpdated = 0;
}


void VirtualMachinePorter::checkVm()
{
    Glib::Mutex::Lock lock(_mutex);
    if (!_vm)
    {
        _vm = _session.getStore().getVmByImportTask(_taskId);
        if (_vm)
        {
            _vm->setBusy(true);
        }
    }
}


RefPtr<VirtualMachine> VirtualMachinePorter::vm()
{
    Glib::Mutex::Lock lock(_mutex);
    return _vm;
}


const char* VirtualMachinePorter::path()
{
    Glib::Mutex::Lock lock(_mutex);
    return _xva ? _xva->path() : NULL;
}


int64_t VirtualMachinePorter::size()
{
    Glib::Mutex::Lock lock(_mutex);
    return _xva ? _xva->size() : -1;
}


int64_t VirtualMachinePorter::nbytes()
{
    Glib::Mutex::Lock lock(_mutex);
    return _xva ? _xva->nbytes() : -1;
}


int VirtualMachinePorter::percent()
{
    Glib::Mutex::Lock lock(_mutex);
    if (_xva)
    {
        int64_t size = _xva->size();
        int64_t nbytes = _xva->nbytes();
        int percent = -1;
        if (size)
        {
            percent = (int)(((nbytes * 1000) / size + 5) / 10);
        }
        return percent;
    }
    else
    {
        return -1;
    }
}
