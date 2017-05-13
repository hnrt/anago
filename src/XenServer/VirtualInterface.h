// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALINTERFACE_H
#define HNRT_VIRTUALINTERFACE_H


#include "XenObject.h"


namespace hnrt
{
    class VirtualInterface
        : public XenObject
    {
    public:

        static RefPtr<VirtualInterface> create(Session&, xen_vif, const XenPtr<xen_vif_record>&);

        virtual ~VirtualInterface();
        XenPtr<xen_vif_record> getRecord() const;
        void setRecord(const XenPtr<xen_vif_record>&);
        RefPtr<VirtualMachine> getVm() const;
        Glib::ustring getDeviceName() const;

    protected:

        VirtualInterface(Session&, xen_vif, const XenPtr<xen_vif_record>&);
        VirtualInterface(const VirtualInterface&);
        void operator =(const VirtualInterface&);

        XenPtr<xen_vif_record> _record;
    };
}


#endif //!HNRT_VIRTUALINTERFACE_H
