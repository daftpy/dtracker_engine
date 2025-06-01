#include <dtracker/audio/playback/tone_playback.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace dtracker::audio::playback
{
    // Initializes the sine wave generator with default frequency of
    // 440 and given sample rate
    TonePlayback::TonePlayback(float sampleRate)
    {
        m_sine.sampleRate = sampleRate;
        m_sine.phase = 0.0f;
    }

    // Initializes the sine wave generator with given frequency and
    // sample rate
    TonePlayback::TonePlayback(float freq, float sampleRate)
    {
        m_sine.frequency = freq;
        m_sine.sampleRate = sampleRate;
        m_sine.phase = 0.0f;
    }

    // Render audio into the output buffer by generating a sine wave
    void TonePlayback::render(float *output, unsigned int nFrames,
                              unsigned int channels)
    {
        const float twoPi = 2.0f * static_cast<float>(M_PI);
        const float phaseInc = twoPi * m_sine.frequency / m_sine.sampleRate;

        // Generate nFrames of audio and fill each channel with the same sample
        // (mono-to-stereo)
        for (unsigned int i = 0; i < nFrames; ++i)
        {
            float sample = std::sin(m_sine.phase);
            m_sine.phase += phaseInc;

            // Wrap phase to avoid overflow
            if (m_sine.phase >= twoPi)
                m_sine.phase -= twoPi;

            // Write the same sample to each channel (e.g., stereo = left/right)
            for (unsigned int ch = 0; ch < channels; ++ch)
                output[i * channels + ch] += sample;
        }
    }

    void TonePlayback::reset()
    {
        // Nothing
        return;
    }

    // Always returns false for now; playback is continuous
    bool TonePlayback::isFinished() const
    {
        return false;
    }

} // namespace dtracker::audio::playback
