// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALBLOCKDEVICE_H
#define HNRT_VIRTUALBLOCKDEVICE_H


#include "XenObject.h"
#include "XenPtr.h"


namespace hnrt
{
    class VirtualMachine;
    class VirtualDiskImage;

    class VirtualBlockDevice
        : public XenObject
    {
    public:

        static RefPtr<VirtualBlockDevice> create(Session&, xen_vbd, const XenPtr<xen_vbd_record>&);

        virtual ~VirtualBlockDevice();
        XenPtr<xen_vbd_record> getRecord();
        void setRecord(const XenPtr<xen_vbd_record>&);
        RefPtr<VirtualMachine> getVm();
        RefPtr<VirtualDiskImage> getVdi();
        Glib::ustring getUserdevice();
        Glib::ustring getDeviceName();

    protected:

        VirtualBlockDevice(Session&, xen_vbd, const XenPtr<xen_vbd_record>&);
        VirtualBlockDevice(const VirtualBlockDevice&);
        void operator =(const VirtualBlockDevice&);

        XenPtr<xen_vbd_record> _record;
    };
}


#endif //!HNRT_VIRTUALBLOCKDEVICE_H
