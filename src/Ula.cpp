#include "Ula.h"

rm::Ula::Ula()
    : _keyLine{ 255, 255, 255, 255, 255, 255, 255, 255 }
    , _tapeEdgeDetected(false)
    , _tapeBitWasFlipped(false)
{}

byte rm::Ula::in(ushort port) {
    byte result = 0xff;
    if ((port & 0x1) == 0) {
        _responded = true;
        if ((port & 0x8000) == 0)
            result &= (byte)_keyLine[7];

        if ((port & 0x4000) == 0)
            result &= (byte)_keyLine[6];

        if ((port & 0x2000) == 0)
            result &= (byte)_keyLine[5];

        if ((port & 0x1000) == 0)
            result &= (byte)_keyLine[4];

        if ((port & 0x800) == 0)
            result &= (byte)_keyLine[3];

        if ((port & 0x400) == 0)
            result &= (byte)_keyLine[2];

        if ((port & 0x200) == 0)
            result &= (byte)_keyLine[1];

        if ((port & 0x100) == 0)
            result &= (byte)_keyLine[0];

        result = (byte)(result & 0x1f); //mask out lower 4 bits
        result = (byte)(result | 0xa0); //set bit 5 & 7 to 1

        if (_tapeEdgeDetected) {
            if (_pulseLevelLow) {
                result &= (~(TAPE_BIT) & 0xff);    //reset is EAR off
            }
            else {
                result |= (TAPE_BIT); //set is EAR on
            }
        }
        else {
            if (_issue2Keyboard) {
                if ((_lastULAOut & (EAR_BIT + MIC_BIT)) == 0) {
                    result &= (~(TAPE_BIT) & 0xff);
                }
                else
                    result |= TAPE_BIT;
            }
            else {
                if ((_lastULAOut & EAR_BIT) == 0) {
                    result &= (~(TAPE_BIT) & 0xff);
                }
                else
                    result |= TAPE_BIT;
            }
        }
    }

    return result;
}

void rm::Ula::out(ushort port, byte val) {
    _responded = false;
    if ((port & 0x1) == 0)    //Even address, so update ULA
    {
        _responded = true;
        _lastULAOut = val;

        //if (BorderColour != (val & BORDER_BIT))
        //    UpdateScreenBuffer(cpu.t_states);

        //needsPaint = true; //useful while debugging as it renders line by line
        _borderColour = val & BORDER_BIT;  //The LSB 3 bits of val hold the border colour
        int beepVal = val & EAR_BIT;

        if (!_tapeEdgeDetected) {

            if (beepVal != _lastBeeperOut) {

                if ((beepVal) == 0) {
                    _beeperOut = MIN_SOUND_VOL;
                }
                else {
                    _beeperOut = MAX_SOUND_VOL;
                }

                if ((val & MIC_BIT) != 0)   //Boost slightly if MIC is on
                    _beeperOut += (short)(MAX_SOUND_VOL * 0.2f);

                _lastBeeperOut = beepVal;
            }
        }
    }
}

void rm::Ula::flipTapePulseLevel() {
    _pulseLevelLow = !_pulseLevelLow;
    _tapeBitWasFlipped = true;
    _beeperOut = _pulseLevelLow ? SHRT_MIN >> 1 : 0;
}
