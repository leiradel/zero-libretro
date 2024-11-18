#include "ZX48k.h"

rm::ZX48k::ZX48k() : Speccy() {
    _model = MachineModel::_48k;
    _interruptPeriod = 32;
    _frameLength = 69888;

    _clockSpeed = 3.50000;

    _contentionTable = new byte[70930];
    _floatingBusTable = new short[70930];

    for (int f = 0; f < 70930; f++) {
        _floatingBusTable[f] = -1;
    }

    _charRows = 24;
    _charCols = 32;
    _screenWidth = 256;
    _screenHeight = 192;
    _borderTopHeight = 48;
    _borderBottomHeight = 56;
    _borderLeftWidth = 48;
    _borderRightWidth = 48;
    _displayStart = 16384;
    _displayLength = 6144;
    _attributeStart = 22528;
    _attributeLength = 768;
    _borderColour = 7;
    _scanLineWidth = _borderLeftWidth + _screenWidth + _borderRightWidth;

    _tstatesPerScanline = 224;
    _tstateAtTop = _borderTopHeight * _tstatesPerScanline;
    _tstateAtBottom = _borderBottomHeight * _tstatesPerScanline;
    _tstateToDisp = new short[_frameLength];

    _screenBuffer = new int[_scanLineWidth * _borderTopHeight //48 lines of border
                                        + _scanLineWidth * _screenHeight //border + main + border of 192 lines
                                        + _scanLineWidth * _borderBottomHeight]; //56 lines of border

    _lastScanlineColor = new int[_scanLineWidth];
    _keyBuffer = new bool[(size_t)KeyCode::LAST];

    _attr = new short[_displayLength]; //6144 bytes of display memory will be mapped
    _lastSoundOut = 0;
    _soundOut = 0;
    _averagedSound = 0;
    _soundCounter = 0;
    _hasAYSound = false;

    reset(true);
}

rm::ZX48k::~ZX48k() {
    delete[] _attr;
    delete[] _keyBuffer;
    delete[] _lastScanlineColor;
    delete[] _screenBuffer;
    delete[] _tstateToDisp;
    delete[] _floatingBusTable;
    delete[] _contentionTable;
}

