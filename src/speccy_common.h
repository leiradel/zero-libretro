#pragma once

namespace rm {
    enum class SPECCY_DEVICE {
        ULA_48K,
        ULA_Pentagon,
        ULA_PLUS,
        KEMPSTON_JOYSTICK,
        SINCLAIR1_JOYSTICK,
        SINCLAIR2_JOYSTICK,
        CURSOR_JOYSTICK,
        KEMPSTON_MOUSE,
        AY_3_8912
    };

    //Matches SZX snapshot machine identifiers
    enum class MachineModel {
        _16k,
        _48k,
        _128k,
        _plus2,
        _plus2A,
        _plus3,
        _plus3E,
        _pentagon,
        _SE = 11,
        _NTSC48k = 15,
        _128ke = 16
    };

    //Each RAM bank consists of 2 8k halves.
    enum class RAM_BANK {
        ZERO_LOW,
        ZERO_HIGH,
        ONE_LOW,
        ONE_HIGH,
        TWO_LOW,
        TWO_HIGH,
        THREE_LOW,
        THREE_HIGH,
        FOUR_LOW,
        FOUR_HIGH,
        FIVE_LOW,
        FIVE_HIGH,
        SIX_LOW,
        SIX_HIGH,
        SEVEN_LOW,
        SEVEN_HIGH,
        EIGHT_LOW,
        EIGHT_HIGH
    };
}
