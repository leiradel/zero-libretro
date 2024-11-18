#pragma once

#include "IODevice.h"
#include "Types.h"

namespace rm {
    class AudioDevice: public IODevice
    {
    public:
        virtual void Update(int deltaTstates) = 0;
        virtual void SampleOutput() = 0;
        virtual void EndSampleFrame() = 0;
        virtual void ResetSamples() = 0;
        virtual void EnableStereoSound(bool is_stereo) = 0;
        virtual void SetChannelsACB(bool is_acb) = 0;

        // We only support stereo or mono sound.
        // If the audio device only supports mono, it sends the same
        // data for both channels.
        int SoundChannel1;
        int SoundChannel2;
    };
}
