#pragma once

#include "Types.h"

#include <limits.h>

namespace rm {
    class Ula {
    public:
        Ula();

        byte in (ushort port);
        void out(ushort port, byte val);

        void flipTapePulseLevel();

    protected:
        static const int BORDER_BIT = 0x07;
        static const int EAR_BIT = 0x10;
        static const int MIC_BIT = 0x08;
        static const int TAPE_BIT = 0x40;
        static const short MIN_SOUND_VOL = 0;
        static const short MAX_SOUND_VOL = SHRT_MAX / 2;

        // Keyboard lines
        int _keyLine[8];
        int _borderColour;

        // Tape management
        bool _pulseLevelLow;
        bool _tapeEdgeDetected;
        bool _tapeBitWasFlipped;

        bool _responded;
        bool _issue2Keyboard;
        int _lastULAOut;
        int _lastBeeperOut;
        int _beeperOut;
    };
}
