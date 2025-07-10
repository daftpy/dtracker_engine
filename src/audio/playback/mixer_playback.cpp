#include <cstring> // for std::memset
#include <dtracker/audio/playback/mixer_playback.hpp>
#include <iostream>

namespace dtracker::audio::playback
{
    void MixerPlaybackUnit::setBufferPool(BufferPool *pool)
    {
        m_bufferPool = pool;
    }

    void MixerPlaybackUnit::setWaveformQueue(
        rigtorp::SPSCQueue<BufferPool::PooledBufferPtr> *queue)
    {
        m_waveformQueue = queue;
    }

    // Adds a new playback unit to the internal list
    void MixerPlaybackUnit::addUnit(std::shared_ptr<PlaybackUnit> unit)
    {
        m_units.push_back(std::move(unit));
    }

    // Mixes all active units into the provided output buffer
    void MixerPlaybackUnit::render(float *buffer, unsigned int nFrames,
                                   unsigned int channels,
                                   const types::RenderContext &context)
    {
        // Start with silence
        std::fill(buffer, buffer + nFrames * channels, 0.0f);

        // Temporary buffer for each unit's output
        std::vector<float> temp(nFrames * channels, 0.0f);

        for (auto it = m_units.begin(); it != m_units.end();)
        {
            std::fill(temp.begin(), temp.end(), 0.0f); // Zero the temp
            (*it)->render(temp.data(), nFrames, channels,
                          context); // Render to temp

            // Mix into main buffer
            for (unsigned int i = 0; i < nFrames * channels; ++i)
                buffer[i] += temp[i];

            // Remove finished units
            if ((*it)->isFinished())
            {
                std::cout << "MixerPlaybackUnit: "
                          << "tracks finished, erasing\n";
                it = m_units.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // --- ADDED: WAVEFORM TAP ---
        // At this point, 'buffer' contains the final, mixed master output.
        if (m_bufferPool && m_waveformQueue)
        {
            // Acquire a recycled buffer from the pool.
            auto transportBuffer = m_bufferPool->acquire();
            if (transportBuffer)
            {
                // Copy this frame's audio into the transport buffer.
                size_t samplesToCopy = std::min((size_t)nFrames * channels,
                                                transportBuffer->size());
                memcpy(transportBuffer->data(), buffer,
                       samplesToCopy * sizeof(float));

                // Push the smart pointer to the buffer onto the lock-free
                // queue.
                // If the push fails (queue is full), the transportBuffer is
                // destroyed here, returning the raw buffer to the pool.
                m_waveformQueue->try_push(std::move(transportBuffer));
            }
        }
    }

    void MixerPlaybackUnit::clear()
    {
        m_units.clear();
    }

    void MixerPlaybackUnit::reset()
    {
        std::cout << "MixerPlaybackUnit: resetting all contained units\n";

        // for (auto *unit : m_units)
        // {
        //     unit->reset();
        // }
    }

    // True if no units are left to play
    bool MixerPlaybackUnit::isFinished() const
    {
        return m_units.empty();
    }

} // namespace dtracker::audio::playback
