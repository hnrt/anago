// Copyright (C) 2012-2017 Hideaki Narita


#include "XenObject.h"


namespace hnrt {


const char* GetXenObjectTypeText(const XenObject& object)
{
    switch (object.getType())
    {
#define CASE(x) case XenObject::x: return #x
    CASE(HOST);
    CASE(HOST_METRICS);
    CASE(NETWORK);
    CASE(PBD);
    CASE(PIF);
    CASE(PATCH);
    CASE(POOL);
    CASE(POOL_PATCH);
    CASE(SESSION);
    CASE(SR);
    CASE(TASK);
    CASE(VBD);
    CASE(VDI);
    CASE(VIF);
    CASE(VM);
    CASE(VM_METRICS);
    CASE(VM_GUEST_METRICS);
    CASE(VM_EXPORTER);
    CASE(VM_IMPORTER);
    CASE(VM_VERIFIER);
    default: return "?";
    }
}


} // namespace hnrt
