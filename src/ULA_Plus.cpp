#include "ULA_Plus.h"
#include "zx_spectrum.h"

void rm::ULA_Plus::RegisterDevice(zx_spectrum* speccyModel) {
    speccyModel->io_devices.erase(this);
    speccyModel->io_devices.insert(this);
    ULAOutEvent = [speccyModel]() -> void {
        speccyModel->UpdateScreenBuffer();
    };
    Enabled = true;
}

void rm::ULA_Plus::UnregisterDevice(zx_spectrum* speccyModel) {
    ULAOutEvent = []() -> void {};
    speccyModel->io_devices.erase(this);
    Enabled = false;
}
