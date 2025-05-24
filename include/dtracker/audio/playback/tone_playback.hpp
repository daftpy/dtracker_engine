#pragma once
#include <cmath>
#include <dtracker/audio/playback/playback_unit.hpp>
#include <dtracker/audio/types/waves/sine_wav.hpp>
#include <memory>

namespace dtracker::audio::playback
{
    class TonePlayback : public PlaybackUnit
    {
      public:
        explicit TonePlayback(float sampleRate);
        explicit TonePlayback(float freq, float sampleRate);

        void render(float *output, unsigned int nFrames,
                    unsigned int channels) override;
        bool isFinished() const override;

      private:
        types::waves::SineWave m_sine;
    };
} // namespace dtracker::audio::playback
