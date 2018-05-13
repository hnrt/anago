// Copyright (C) 2012-2018 Hideaki Narita


#define NO_TRACE


#include <errno.h>
#include <string.h>
#include <glibmm/ustring.h>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Session.h"
#include "VirtualMachineArchive.h"
#include "VirtualMachineVerifier.h"


using namespace hnrt;


RefPtr<VirtualMachineVerifier> VirtualMachineVerifier::create()
{
    return RefPtr<VirtualMachineVerifier>(new VirtualMachineVerifier);
}


VirtualMachineVerifier::VirtualMachineVerifier()
    : VirtualMachinePorter(VM_VERIFIER, *(new Session), "VirtualMachineVerifier")
{
    TRACEFUN(this, "VirtualMachineVerifier::ctor");
}


VirtualMachineVerifier::~VirtualMachineVerifier()
{
    TRACEFUN(this, "VirtualMachineVerifier::dtor");
    _session.decRef();
}


void VirtualMachineVerifier::run(const char* path)
{
    TRACEFUN(this, "VirtualMachineVerifier::run(%s)", path);

    init(path);

    try
    {
        if (!_xva->open())
        {
            throw StringBuffer().format("%s: %s", strerror(_xva->error()), _xva->path());
        }

        emit(XenObject::VERIFY_PENDING);

        _state = VirtualMachineOperationState::VERIFY_INPROGRESS;

        if (_xva->validate(_abort))
        {
            Logger::instance().info("Verified %'zu bytes: %s", _xva->nbytes(), _xva->path());
            _state = VirtualMachineOperationState::VERIFY_SUCCESS;
            emit(XenObject::VERIFIED);
        }
        else if (_xva->error() == ECANCELED)
        {
            Logger::instance().info("Verify canceled: %s", _xva->path());
            _state = VirtualMachineOperationState::VERIFY_CANCELED;
            emit(XenObject::VERIFY_CANCELED);
        }
        else if (_xva->error() == EPROTO)
        {
            throw StringBuffer().format("Stopped at %'zu: %s", _xva->nbytes(), _xva->path());
        }
        else
        {
            throw StringBuffer().format("%s: %s", strerror(_xva->error()), _xva->path());
        }
    }
    catch (StringBuffer msg)
    {
        Logger::instance().warn("Verify failed: %s", msg.str());
        _state = VirtualMachineOperationState::VERIFY_FAILURE;
        emit(XenObject::VERIFY_FAILED);
    }
    catch (...)
    {
        Logger::instance().warn("Unhandled exception caught.");
        _state = VirtualMachineOperationState::VERIFY_FAILURE;
        emit(XenObject::VERIFY_FAILED);
    }
}


void VirtualMachineVerifier::init(const char* path)
{
    Glib::Mutex::Lock lock(_mutex);
    VirtualMachinePorter::init(path, "r", VirtualMachineOperationState::VERIFY_PENDING);
}
