#pragma once

#include <dtracker/audio/playback/playback_unit.hpp>
#include <vector>

namespace dtracker::audio::playback
{

    class SamplePlaybackUnit : public PlaybackUnit
    {
      public:
        // Playback class from interleaved PCM float data
        SamplePlaybackUnit(std::vector<float> samples, unsigned int sampleRate);

        void render(float *buffer, unsigned int frames,
                    unsigned int channels) override;
        bool isFinished() const override;

        void reset() override; // Resets playback to the beginning

        // Returns pcm Data
        const std::vector<float> &data() const;

        unsigned int sampleRate() const;

      private:
        std::vector<float> m_samples; // Interleaved stereo data (L R L R... )
        size_t m_position = 0;        // Current sample index (not byte index)
        unsigned int m_sampleRate;    // For timing conversions
    };

} // namespace dtracker::audio::playback
