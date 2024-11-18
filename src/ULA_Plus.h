#pragma once

#include "IODevice.h"
#include "Types.h"

#include <functional>

namespace rm {
    class zx_spectrum;

    class ULA_Plus : public IODevice
    {
    public:
        std::function<void()> ULAOutEvent;

        bool _Responded = false;
        virtual bool Responded() override { return _Responded; }
        void Responded(bool x) { _Responded = x; }

        // Following values taken from generic colour palette from ULA plus site
        int Palette[64] = { 0x000000, 0x404040, 0xff0000,0xff6a00,0xffd800,0xb6ff00,0x4cff00,0x00ff21,
                                                    0x00ff90,0x00ffff,0x0094ff,0x0026ff,0x4800ff,0xb200ff,0xff00dc,0xff006e,
                                                    0xffffff,0x808080,0x7f0000,0x7f3300,0x7f6a00,0x5b7f00,0x267f00,0x007f0e,
                                                    0x007f46,0x007f7f,0x004a7f,0x00137f,0x21007f,0x57007f,0x7f006e,0x7f0037,
                                                    0xa0a0a0,0x303030,0xff7f7f,0xffb27f,0xffe97f,0xdaff7f,0xa5ff7f,0x7fff8e,
                                                    0x7fffc5,0x7fffff,0x7fc9ff,0x7f92ff,0xa17fff,0xd67fff,0xff7fed,0xff7fb6,
                                                    0xc0c0c0,0x606060,0x7f3f3f,0x7f593f,0x7f743f,0x6d7f3f,0x527f3f,0x3f7f47,
                                                    0x3f7f62,0x3f7f7f,0x3f647f,0x3f497f,0x503f7f,0x6b3f7f,0x7f3f76,0x7f3f5b
                                                  };

        bool Enabled = false;
        int GroupMode = 0; //0 = palette group, 1 = mode group
        int PaletteGroup = 0;
        bool PaletteEnabled = false;
        
        virtual SPECCY_DEVICE DeviceID() override { return SPECCY_DEVICE::ULA_PLUS; }
        byte lastULAPlusOut = 0;

        virtual byte In(ushort port) override {
            byte result = 0xff;
            _Responded = false;
            if (Enabled && port == 0xff3b) {
                _Responded = true;
                result = lastULAPlusOut;
            }
            return result;
        }

        virtual void Out(ushort port, byte val) override {
            _Responded = false;
            if (Enabled) {
                if (port == 0xbf3b) {
                    _Responded = true;
                    int mode = (val & 0xc0) >> 6;

                    //mode group
                    if (mode == 1) {
                        GroupMode = 1;
                    }
                    else if (mode == 0) //palette group
                    {
                        GroupMode = 0;
                        PaletteGroup = val & 0x3f;
                    }

                }
                else if (port == 0xff3b) {
                    _Responded = true;

                    if (lastULAPlusOut != val) {
                        ULAOutEvent();
                    }

                    lastULAPlusOut = val;

                    if (GroupMode == 1) {
                        PaletteEnabled = (val & 0x01) != 0;
                    }
                    else {
                        // code below by evolutional(discord).
                        int r = (val & 0b00011100) >> 2;
                        int g = (val & 0b11100000) >> 5;
                        int b = (val & 0b00000011) << 1;

                        if (b != 0) {
                            b |= 0x1;
                        }

                        r = (r << 5) | (r << 2) | (r >> 1);
                        g = (g << 5) | (g << 2) | (g >> 1);
                        b = (b << 5) | (b << 2) | (b >> 1);
                        Palette[PaletteGroup] = r << 16 | g << 8 | b;
                    }
                }
            }
        }

        virtual void RegisterDevice(zx_spectrum* speccyModel) override;

        virtual void Reset() override {
            lastULAPlusOut = 0;
            GroupMode = 0;
            PaletteGroup = 0;
            PaletteEnabled = false;
        }

        virtual void UnregisterDevice(zx_spectrum* speccyModel) override;
    }
}
