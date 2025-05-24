#pragma once

#include <dtracker/audio/playback/playback_unit.hpp>
#include <vector>

namespace dtracker::audio::playback
{

    class SamplePlayback : public PlaybackUnit
    {
      public:
        // Playback class from interleaved PCM float data
        SamplePlayback(std::vector<float> samples, unsigned int sampleRate);

        void render(float *buffer, unsigned int frames,
                    unsigned int channels) override;
        bool isFinished() const override;

      private:
        std::vector<float> m_samples; // Interleaved stereo data (L R L R... )
        size_t m_position = 0;        // Current sample index (not byte index)
        unsigned int m_sampleRate;    // For timing conversions
    };

} // namespace dtracker::audio::playback
