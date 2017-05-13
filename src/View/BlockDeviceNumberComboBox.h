// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_BLOCKDEVICENUMBERCOMBOBOX_H
#define HNRT_BLOCKDEVICENUMBERCOMBOBOX_H


#include "DeviceNumberComboBox.h"


namespace hnrt
{
    class VirtualMachine;

    class BlockDeviceNumberComboBox
        : public DeviceNumberComboBox
    {
    public:

        BlockDeviceNumberComboBox(const VirtualMachine&);
        virtual ~BlockDeviceNumberComboBox();

    private:

        BlockDeviceNumberComboBox(const BlockDeviceNumberComboBox&);
        void operator =(const BlockDeviceNumberComboBox&);
        void initStore(const VirtualMachine&);
    };
}


#endif //!HNRT_BLOCKDEVICENUMBERCOMBOBOX_H
