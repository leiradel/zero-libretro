#pragma once

#include "speccy_common.h"
#include "Types.h"

namespace rm {
    class zx_spectrum;

    class SpeccyDevice
    {
    public:
        virtual void RegisterDevice(zx_spectrum* speccyModel) = 0;
        virtual void UnregisterDevice(zx_spectrum* speccyModel) = 0;
        virtual void Reset() = 0;
        virtual SPECCY_DEVICE DeviceID() = 0;
    }
}
