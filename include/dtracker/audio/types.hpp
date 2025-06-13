#pragma once
#include <vector>

namespace dtracker::audio::types
{
    using PCMData = std::vector<float>;

    // For the engine audio stream
    struct AudioSettings
    {
        unsigned int sampleRate = 44100;
        unsigned int bufferFrames = 512;
        unsigned int outputChannels = 2;
    };

    // For sample files
    struct AudioProperties
    {
        unsigned int sampleRate;
        unsigned int bitDepth;
        unsigned int numChannels;
    };
} // namespace dtracker::audio::types