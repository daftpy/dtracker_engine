#include <algorithm> // For std::fill
#include <cmath>     // For std::max/min
#include <dtracker/audio/playback/track_playback_unit.hpp>
#include <iostream>

namespace dtracker::audio::playback
{
    TrackPlaybackUnit::TrackPlaybackUnit(
        BufferPool *bufferPool,
        rigtorp::SPSCQueue<BufferPool::PooledBufferPtr> *waveformQueue)
        : m_bufferPool(bufferPool), m_waveformQueue(waveformQueue)
    {
    }

    // Renamed from addSample to addUnit to be more generic.
    void TrackPlaybackUnit::addUnit(std::unique_ptr<PatternPlaybackUnit> unit)
    {
        if (unit)
            m_units.push_back(std::move(unit));
    }

    void TrackPlaybackUnit::setVolume(float v)
    {
        m_volume = std::clamp(v, 0.0f, 1.0f);
    }

    void TrackPlaybackUnit::setPan(float p)
    {
        m_pan = std::clamp(p, -1.0f, 1.0f);
    }

    void TrackPlaybackUnit::render(float *buffer, unsigned int nFrames,
                                   unsigned int channels,
                                   const types::RenderContext &context)
    {
        // Your existing render logic to play the sequence and apply vol/pan
        // comes first. It correctly fills the `buffer` with processed audio.
        if (isFinished() || m_units.empty())
        {
            std::fill(buffer, buffer + nFrames * channels, 0.0f);
            return;
        }

        auto &currentUnit = m_units[m_currentUnitIndex];
        currentUnit->render(buffer, nFrames, channels, context);

        if (channels == 2)
        {
            float leftGain = m_volume * (1.0f - std::max(0.0f, m_pan));
            float rightGain = m_volume * (1.0f + std::min(0.0f, m_pan));
            for (unsigned int i = 0; i < nFrames; ++i)
            {
                buffer[i * 2] *= leftGain;
                buffer[i * 2 + 1] *= rightGain;
            }
        }

        // TODO: Implement the waveform tap for individual tracks.
        // Currently, enabling this exhausts the buffer pool too quickly.
        // This is because a resource leak. The buffers acquired by the
        // TrackPlaybackUnits are pushed into queues that are never emptied.
        // After a few seconds, all 128 buffers from the shared BufferPool
        // are stuck inside these unread queues. The pool runs dry,
        // m_bufferPool->acquire() starts returning nullptr, and the waveform
        // tap in the MixerPlaybackUnit stops working because it can't get any
        // more empty buffers.

        // --- WAVEFORM TAP ---
        // At this point, 'buffer' contains the final processed audio.
        // if (m_bufferPool && m_waveformQueue)
        // {
        //     // Acquire a recycled buffer from the pool.
        //     auto transportBuffer = m_bufferPool->acquire();
        //     if (transportBuffer)
        //     {
        //         // Copy this frame's audio into the transport buffer.
        //         size_t samplesToCopy = std::min((size_t)nFrames * channels,
        //                                         transportBuffer->size());
        //         memcpy(transportBuffer->data(), buffer,
        //                samplesToCopy * sizeof(float));

        //         // Push the smart pointer to the buffer onto the lock-free
        //         // queue.
        //         // If the push fails (queue is full), the transportBuffer is
        //         // destroyed here, returning the raw buffer to the pool.
        //         m_waveformQueue->try_push(std::move(transportBuffer));
        //     }
        // }

        // Handle looping behavior
        if (currentUnit->hasFinishedLoop())
        {
            if (context.isLooping)
            {
                // Reset the current pattern and go back to the first pattern
                currentUnit->reset();
                m_currentUnitIndex = 0;
            }
            else if (m_currentUnitIndex < m_units.size() - 1)
            {
                // Reset the current pattern and move onto the next pattern
                currentUnit->reset();
                m_currentUnitIndex++;
            }
        }
    }

    void TrackPlaybackUnit::reset()
    {
        // When the track is reset, go back to the first pattern in the
        // sequence.
        m_currentUnitIndex = 0;
        // Also reset all the patterns it contains.
        for (auto &unit : m_units)
        {
            if (unit)
                unit->reset();
        }
    }

    bool TrackPlaybackUnit::isFinished() const
    {
        // The track is finished once it has played all the patterns in its
        // sequence.
        return m_currentUnitIndex >= m_units.size() - 1 &&
               m_units[m_currentUnitIndex]
                   ->isFinished(); // ensure the current pattern is done
                                   // rendering before reporting finished
    }

} // namespace dtracker::audio::playback