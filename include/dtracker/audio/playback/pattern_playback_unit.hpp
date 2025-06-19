#pragma once

#include <dtracker/audio/playback/playback_unit.hpp>
#include <dtracker/audio/playback/unit_pool.hpp>
#include <dtracker/sample/types.hpp>
#include <dtracker/tracker/types.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

namespace dtracker::audio::playback
{
    /// A map of Sample IDs to their fully resolved sample data.
    using SampleBlueprint =
        std::unordered_map<int, dtracker::sample::types::SampleDescriptor>;

    /// A real-time safe sequencer that schedules and mixes the notes for a
    /// single pattern. It is given all necessary data upon construction and has
    /// no external dependencies during its real-time render loop.
    class PatternPlaybackUnit : public PlaybackUnit
    {
      public:
        /// Constructs the pattern player.
        /// @param pattern The sequence of sample IDs and timing information.
        /// @param blueprint A map containing all necessary sample data for fast
        /// lookups.
        /// @param sampleUnitPool A pointer to the object pool for recycling
        /// players.
        /// @param sampleRate The project's sample rate, for accurate timing
        /// calculations.
        PatternPlaybackUnit(
            const dtracker::tracker::types::ActivePattern &pattern,
            const SampleBlueprint &blueprint, UnitPool *sampleUnitPool,
            unsigned int sampleRate);

        /// Performs one block of processing, scheduling new notes and mixing
        /// active ones.
        void render(float *buffer, unsigned int nFrames,
                    unsigned int channels) override;

        /// Returns true once the pattern has played through one full cycle
        /// AND all of its triggered notes have finished their playback.
        bool isFinished() const override;

        virtual bool hasFinishedLoop() const;

        /// Resets the pattern to its initial state, ready to be played again.
        void reset() override;

      private:
        /// The 'sheet music' for this pattern, including steps and timing
        /// state.
        dtracker::tracker::types::ActivePattern m_pattern;

        /// A local copy of all sample data needed by this pattern, for fast,
        /// lock-free lookups.
        SampleBlueprint m_blueprint;

        /// A non-owning pointer to the central pool for acquiring recycled
        /// player objects.
        UnitPool *m_sampleUnitPool;

        /// The audio engine's sample rate, used for accurate time calculations.
        unsigned int m_sampleRate;

        /// A state flag that becomes true after the sequencer has played all
        /// steps once.
        bool m_hasFinishedOneLoop{false};

        /// This pattern's internal mixer; holds all notes that were triggered
        /// and are currently playing.
        std::vector<UnitPool::PooledUnitPtr> m_activeNotes;
    };
} // namespace dtracker::audio::playback