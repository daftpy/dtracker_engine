#pragma once
#include <vector>

namespace dtracker::audio::types
{
    using PCMData = std::vector<float>;

    struct AudioProperties
    {
        unsigned int sampleRate;
        unsigned int bitDepth;
        unsigned int numChannels;
    };
} // namespace dtracker::audio::types