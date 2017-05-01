// Copyright (C) 2012-2017 Hideaki Narita


//#define NO_TRACE


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
    TRACE("VirtualMachineVerifier::ctor");
}


VirtualMachineVerifier::~VirtualMachineVerifier()
{
    TRACE("VirtualMachineVerifier::dtor");
    _session.decRef();
}


void VirtualMachineVerifier::run(const char* path)
{
    TRACE("VirtualMachineVerifier::run", "path=\"%s\"", path);

    open(path);

    try
    {
        if (!_xva->open())
        {
            throw StringBuffer().format("%s: %s", _xva->path(), strerror(_xva->error()));
        }

        emit(XenObject::VERIFY_PENDING);

        _state = VirtualMachineOperationState::VERIFY_INPROGRESS;

        if (_xva->validate(_abort))
        {
            Logger::instance().info("%s: %'zu bytes verified.", _xva->path(), _xva->nbytes());
            _state = VirtualMachineOperationState::VERIFY_SUCCESS;
            emit(XenObject::VERIFIED);
        }
        else if (_xva->error() == ECANCELED)
        {
            Logger::instance().info("%s: Verify canceled.", _xva->path());
            _state = VirtualMachineOperationState::VERIFY_CANCELED;
            emit(XenObject::VERIFY_CANCELED);
        }
        else if (_xva->error() == EPROTO)
        {
            throw StringBuffer().format("%s: Verify failed at %'zu bytes.", _xva->path(), _xva->nbytes());
        }
        else
        {
            throw StringBuffer().format("%s: %s", _xva->path(), strerror(_xva->error()));
        }
    }
    catch (StringBuffer msg)
    {
        Logger::instance().warn("VirtualMachineVerifier: %s", msg.str());
        _state = VirtualMachineOperationState::VERIFY_FAILURE;
        emit(XenObject::VERIFY_FAILED);
    }
    catch (...)
    {
        Logger::instance().warn("VirtualMachineVerifier: Unhandled exception caught.");
        _state = VirtualMachineOperationState::VERIFY_FAILURE;
        emit(XenObject::VERIFY_FAILED);
    }

    close();
}


void VirtualMachineVerifier::open(const char* path)
{
    Glib::Mutex::Lock lock(_mutex);
    VirtualMachinePorter::open(path, "r", VirtualMachineOperationState::VERIFY_PENDING);
}


void VirtualMachineVerifier::close()
{
    Glib::Mutex::Lock lock(_mutex);
    VirtualMachinePorter::close();
}
