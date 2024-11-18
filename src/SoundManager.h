#pragma once

#include "Types.h"

namespace rm {
    class SoundManager
    {
    public:
        void SetVolume(float) {}
        void Init(IntPtr, short, short, int) {}
        void Reset() {}
        void ChangeSampleSize(int) {}
        void Play() {}
        void Stop() {}
        void Shutdown() {}
        bool FinishedPlaying() { return true; }
    };
}
