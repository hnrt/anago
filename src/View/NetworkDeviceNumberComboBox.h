// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_NETWORKDEVICENUMBERCOMBOBOX_H
#define HNRT_NETWORKDEVICENUMBERCOMBOBOX_H


#include "DeviceNumberComboBox.h"


namespace hnrt
{
    class VirtualMachine;

    class NetworkDeviceNumberComboBox
        : public DeviceNumberComboBox
    {
    public:

        NetworkDeviceNumberComboBox(const VirtualMachine&);
        virtual ~NetworkDeviceNumberComboBox();

    private:

        NetworkDeviceNumberComboBox(const NetworkDeviceNumberComboBox&);
        void operator =(const NetworkDeviceNumberComboBox&);
        void initStore(const VirtualMachine&);
    };
}


#endif //!HNRT_NETWORKDEVICENUMBERCOMBOBOX_H
