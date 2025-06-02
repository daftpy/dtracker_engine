#pragma once

#include <dtracker/audio/playback/playback_unit.hpp>
#include <memory>
#include <vector>

namespace dtracker::audio::playback
{
    // MixerPlaybackUnit allows multiple PlaybackUnits to play simultaneously.
    // It mixes their outputs together and automatically removes finished units.
    class MixerPlaybackUnit : public PlaybackUnit
    {
      public:
        MixerPlaybackUnit() = default;

        // Adds a new playback unit to be mixed with others
        void addUnit(std::unique_ptr<PlaybackUnit> unit);

        // Renders audio by mixing all active units into the output buffer
        void render(float *buffer, unsigned int nFrames,
                    unsigned int channels) override;

        void reset() override;

        void clear();

        // Returns true if no active units remain (i.e. playback is silent)
        bool isFinished() const override;

      private:
        // Holds all active playback units being mixed
        std::vector<std::unique_ptr<PlaybackUnit>> m_units;
    };
} // namespace dtracker::audio::playback
