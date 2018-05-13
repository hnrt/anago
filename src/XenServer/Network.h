// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_NETWORK_H
#define HNRT_NETWORK_H


#include "XenObject.h"


namespace hnrt
{
    class Network
        : public XenObject
    {
    public:

        static RefPtr<Network> create(Session&, xen_network, const XenPtr<xen_network_record>&);

        virtual ~Network();
        XenPtr<xen_network_record> getRecord() const;
        void setRecord(const XenPtr<xen_network_record>&);
        bool setName(const char* label, const char* description);
        RefPtr<VirtualMachine> getVm(size_t = 0) const;
        bool isHostInternalManagement() const;

    protected:

        Network(Session&, xen_network, const XenPtr<xen_network_record>&);
        Network(const Network&);
        void operator =(const Network&);

        XenPtr<xen_network_record> _record;
    };
}


#endif //!HNRT_NETWORK_H
