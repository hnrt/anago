// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PHYSICALBLOCKDEVICE_H
#define HNRT_PHYSICALBLOCKDEVICE_H


#include "XenObject.h"
#include "XenPtr.h"


namespace hnrt
{
    class StorageRepository;

    class PhysicalBlockDevice
        : public XenObject
    {
    public:

        static RefPtr<PhysicalBlockDevice> create(Session&, xen_pbd, const XenPtr<xen_pbd_record>&);

        XenPtr<xen_pbd_record> getRecord();
        void setRecord(const XenPtr<xen_pbd_record>&);
        RefPtr<StorageRepository> getSr();

    protected:

        PhysicalBlockDevice(Session&, xen_pbd, const XenPtr<xen_pbd_record>&);
        PhysicalBlockDevice(const PhysicalBlockDevice&);
        void operator =(const PhysicalBlockDevice&);

        XenPtr<xen_pbd_record> _record;
    };
}


#endif //!HNRT_PHYSICALBLOCKDEVICE_H
