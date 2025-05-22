#pragma once

namespace dtracker::audio::types::waves
{
    struct SineWave
    {
        float phase = 0.0f;
        float frequency = 440.0f; // A4
        float sampleRate = 44100.0f;
    };
} // namespace dtracker::audio::types::waves
