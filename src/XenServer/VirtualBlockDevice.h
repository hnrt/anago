// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_VIRTUALBLOCKDEVICE_H
#define HNRT_VIRTUALBLOCKDEVICE_H


#include "XenObject.h"


namespace hnrt
{
    class VirtualBlockDevice
        : public XenObject
    {
    public:

        static RefPtr<VirtualBlockDevice> create(Session&, xen_vbd, const XenPtr<xen_vbd_record>&);

        virtual ~VirtualBlockDevice();
        XenPtr<xen_vbd_record> getRecord() const;
        void setRecord(const XenPtr<xen_vbd_record>&);
        RefPtr<VirtualMachine> getVm() const;
        RefPtr<VirtualDiskImage> getVdi() const;
        Glib::ustring getUserdevice() const;
        Glib::ustring getDeviceName() const;

    protected:

        VirtualBlockDevice(Session&, xen_vbd, const XenPtr<xen_vbd_record>&);
        VirtualBlockDevice(const VirtualBlockDevice&);
        void operator =(const VirtualBlockDevice&);

        XenPtr<xen_vbd_record> _record;
    };
}


#endif //!HNRT_VIRTUALBLOCKDEVICE_H
