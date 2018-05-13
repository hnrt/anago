// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_PHYSICALINTERFACE_H
#define HNRT_PHYSICALINTERFACE_H


#include "XenObject.h"


namespace hnrt
{
    class PhysicalInterface
        : public XenObject
    {
    public:

        static RefPtr<PhysicalInterface> create(Session&, xen_pif, const XenPtr<xen_pif_record>&);

        virtual ~PhysicalInterface();
        XenPtr<xen_pif_record> getRecord() const;
        void setRecord(const XenPtr<xen_pif_record>&);

    protected:

        PhysicalInterface(Session&, xen_pif, const XenPtr<xen_pif_record>&);
        PhysicalInterface(const PhysicalInterface&);
        void operator =(const PhysicalInterface&);

        XenPtr<xen_pif_record> _record;
    };
}


#endif //!HNRT_PHYSICALINTERFACE_H
