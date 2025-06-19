#pragma once

#include <dtracker/audio/playback/pattern_playback_unit.hpp>
#include <dtracker/audio/playback/playback_unit.hpp>
#include <memory>
#include <vector>

namespace dtracker::audio::playback
{
    /// Plays a sequence of other PlaybackUnits (like patterns) one after
    /// another. Also applies track-level volume and panning to the output.
    class TrackPlaybackUnit : public PlaybackUnit
    {
      public:
        TrackPlaybackUnit() = default;

        /// Adds a new unit (e.g., a pattern) to the end of the playback
        /// sequence.
        void addUnit(std::unique_ptr<PatternPlaybackUnit> unit);

        /// Sets the track's master volume [0.0 - 1.0].
        void setVolume(float v);

        /// Sets the track's stereo pan [-1.0 (L) to 1.0 (R)].
        void setPan(float p);

        void setLooping(bool shouldLoop);

        // --- Overridden virtual functions ---
        void render(float *buffer, unsigned int nFrames,
                    unsigned int channels) override;
        void reset() override;
        bool isFinished() const override;

      private:
        float m_volume = 1.0f;
        float m_pan = 0.0f;

        // The sequence of units (patterns) to play in order.
        std::vector<std::unique_ptr<PatternPlaybackUnit>> m_units;

        // The index of the pattern in the m_units vector that is currently
        // playing.
        size_t m_currentUnitIndex{0};

        bool m_isLooping{false};
    };
} // namespace dtracker::audio::playback