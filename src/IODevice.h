#pragma once

#include "SpeccyDevice.h"
#include "Types.h"

namespace rm {
    class IODevice: public SpeccyDevice {
    public:
        virtual bool Responded() = 0;
        virtual byte In(ushort port) = 0;
        virtual void Out(ushort port, byte val) = 0;
    }
}
