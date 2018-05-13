// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_VIRTUALDISKIMAGE_H
#define HNRT_VIRTUALDISKIMAGE_H


#include "XenObject.h"
#include "XenPtr.h"


namespace hnrt
{
    class VirtualDiskImage
        : public XenObject
    {
    public:

        static RefPtr<VirtualDiskImage> create(Session&, xen_vdi, const XenPtr<xen_vdi_record>&);

        virtual ~VirtualDiskImage();
        XenPtr<xen_vdi_record> getRecord() const;
        void setRecord(const XenPtr<xen_vdi_record>&);
        RefPtr<StorageRepository> getSr() const;
        RefPtr<VirtualMachine> getVm(size_t = 0) const;
        bool setName(const char* label, const char* description);
        bool destroy();
        bool resize(int64_t);

    protected:

        VirtualDiskImage(Session&, xen_vdi, const XenPtr<xen_vdi_record>&);
        VirtualDiskImage(const VirtualDiskImage&);
        void operator =(const VirtualDiskImage&);

        XenPtr<xen_vdi_record> _record;
    };
}


#endif //!HNRT_VIRTUALDISKIMAGE_H
