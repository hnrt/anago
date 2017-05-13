// Copyright (C) 2012-2017 Hideaki Narita


#include <string.h>
#include <map>
#include "XenObjectTypeMap.h"


namespace hnrt
{
    struct comparator
    {
        bool operator ()(const char* s1, const char* s2)
        {
            return strcmp(s1, s2) < 0;
        }
    };
}


using namespace hnrt;


typedef std::map<const char*, XenObject::Type, comparator> Map;
typedef std::map<const char*, XenObject::Type, comparator>::iterator Iter;
typedef std::map<const char*, XenObject::Type, comparator>::const_iterator ConstIter;
typedef std::pair<const char*, XenObject::Type> Entry;


static Map theMap;


void XenObjectTypeMap::init()
{
    theMap.insert(Entry("host", XenObject::HOST));
    theMap.insert(Entry("host_metrics", XenObject::HOST_METRICS));
    theMap.insert(Entry("network", XenObject::NETWORK));
    theMap.insert(Entry("pbd", XenObject::PBD));
    theMap.insert(Entry("pif", XenObject::PIF));
    theMap.insert(Entry("pool", XenObject::POOL));
    theMap.insert(Entry("pool_patch", XenObject::POOL_PATCH));
    theMap.insert(Entry("session", XenObject::SESSION));
    theMap.insert(Entry("sr", XenObject::SR));
    theMap.insert(Entry("task", XenObject::TASK));
    theMap.insert(Entry("vbd", XenObject::VBD));
    theMap.insert(Entry("vdi", XenObject::VDI));
    theMap.insert(Entry("vif", XenObject::VIF));
    theMap.insert(Entry("vm", XenObject::VM));
    theMap.insert(Entry("vm_metrics", XenObject::VM_METRICS));
    theMap.insert(Entry("vm_guest_metrics", XenObject::VM_GUEST_METRICS));
}


void XenObjectTypeMap::fini()
{
    theMap.clear();
}


XenObject::Type XenObjectTypeMap::find(const char* key)
{
    Iter iter = theMap.find(key);
    if (iter != theMap.end())
    {
        return iter->second;
    }
    else
    {
        return XenObject::NONE;
    }
}


const char* XenObjectTypeMap::toString(XenObject::Type key)
{
    for (ConstIter iter = theMap.begin(); iter != theMap.end(); iter++)
    {
        if (iter->second == key)
        {
            return iter->first;
        }
    }
    return "(undefined)";
}
