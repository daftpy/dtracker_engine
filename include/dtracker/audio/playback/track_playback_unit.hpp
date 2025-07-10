#pragma once

#include <rigtorp/SPSCQueue.h>

#include <dtracker/audio/playback/buffer_pool.hpp>
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
        TrackPlaybackUnit(
            BufferPool *bufferPool,
            rigtorp::SPSCQueue<BufferPool::PooledBufferPtr> *waveformQueue);

        /// Adds a new unit (e.g., a pattern) to the end of the playback
        /// sequence.
        void addUnit(std::unique_ptr<PatternPlaybackUnit> unit);

        /// Sets the track's master volume [0.0 - 1.0].
        void setVolume(float v);

        /// Sets the track's stereo pan [-1.0 (L) to 1.0 (R)].
        void setPan(float p);

        // --- Overridden virtual functions ---
        void render(float *buffer, unsigned int nFrames, unsigned int channels,
                    const types::RenderContext &context) override;
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

        /// A non-owning pointer to the pool of recycled audio buffers.
        BufferPool *m_bufferPool{nullptr};

        /// A non-owning pointer to the thread-safe queue for this specific
        /// track.
        rigtorp::SPSCQueue<BufferPool::PooledBufferPtr> *m_waveformQueue{
            nullptr};
    };
} // namespace dtracker::audio::playback