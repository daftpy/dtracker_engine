#pragma once

namespace dtracker::engine
{
    struct AudioSettings
    {
        unsigned int sampleRate = 44100;
        unsigned int bufferFrames = 512;
        unsigned int outputChannels = 2;
    };
} // namespace dtracker::engine