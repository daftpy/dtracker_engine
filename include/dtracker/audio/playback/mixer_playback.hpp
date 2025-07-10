#pragma once

#include <rigtorp/SPSCQueue.h> // Include SPSCQueue

#include <dtracker/audio/playback/buffer_pool.hpp> // Include BufferPool
#include <dtracker/audio/playback/playback_unit.hpp>
#include <dtracker/audio/types.hpp>
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
        virtual void addUnit(std::shared_ptr<PlaybackUnit> unit);

        // Renders audio by mixing all active units into the output buffer
        void render(float *buffer, unsigned int nFrames, unsigned int channels,
                    const types::RenderContext &context) override;

        void reset() override;

        // Clears the playback units from the mix
        virtual void clear();

        // Returns true if no active units remain (i.e. playback is silent)
        bool isFinished() const override;

        /// Sets the buffer pool to use for acquiring transport buffers.
        void setBufferPool(BufferPool *pool);

        /// Sets the queue to push waveform data to for visualization.
        void setWaveformQueue(
            rigtorp::SPSCQueue<BufferPool::PooledBufferPtr> *queue);

      private:
        // Holds all active playback units being mixed
        std::vector<std::shared_ptr<PlaybackUnit>> m_units;

        /// A non-owning pointer to the pool of recycled audio buffers.
        BufferPool *m_bufferPool{nullptr};

        /// A non-owning pointer to the thread-safe queue for the master output.
        rigtorp::SPSCQueue<BufferPool::PooledBufferPtr> *m_waveformQueue{
            nullptr};
    };
} // namespace dtracker::audio::playback
