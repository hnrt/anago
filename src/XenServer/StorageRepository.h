// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_STORAGEREPOSITORY_H
#define HNRT_STORAGEREPOSITORY_H


#include "XenObject.h"
#include "XenPtr.h"


namespace hnrt
{
    class PhysicalBlockDevice;

    class StorageRepository
        : public XenObject
    {
    public:

        enum SubType
        {
            NONE,
            ANY,
            USR,
            DEV,
            ISO,
        };

        static RefPtr<StorageRepository> create(Session&, xen_sr, const XenPtr<xen_sr_record>&);

        virtual int setBusy(bool = true);
        SubType getSubType() const { return _subType; }
        XenPtr<xen_sr_record> getRecord();
        void setRecord(const XenPtr<xen_sr_record>&);
        RefPtr<PhysicalBlockDevice> getPbd();
        bool setName(const char* name, const char* desc);
        bool isCifs();
        bool isTools();
        bool isDefault();
        bool remove();

    protected:

        StorageRepository(Session&, xen_sr, const XenPtr<xen_sr_record>&);
        StorageRepository(const StorageRepository&);
        void operator =(const StorageRepository&);
        void setStatus();

        SubType _subType;
        XenPtr<xen_sr_record> _record;
    };
}


#endif //!HNRT_STORAGEREPOSITORY_H
