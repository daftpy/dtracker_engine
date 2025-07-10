#include <dtracker/audio/playback/buffer_pool.hpp>
#include <stdexcept>

namespace dtracker::audio::playback
{
    BufferPool::BufferPool(size_t numBuffers, size_t bufferSize)
        : m_bufferSize(bufferSize) // Store the desired size of each buffer
    {
        if (numBuffers == 0 || bufferSize == 0)
            throw std::invalid_argument(
                "Buffer pool and buffer size must be non-zero.");

        m_pool.reserve(numBuffers);
        m_freeList.reserve(numBuffers);

        for (size_t i = 0; i < numBuffers; ++i)
        {
            // Instead of default-constructing, we create each vector
            // with the specified size, pre-filled with 0.0f.
            m_pool.emplace_back(m_bufferSize, 0.0f);
            m_freeList.push_back(&m_pool[i]);
        }
    }

    // acquire() and release() are IDENTICAL in logic to UnitPool,
    // just with different types.
    BufferPool::PooledBufferPtr BufferPool::acquire()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_freeList.empty())
        {
            return nullptr;
        }
        auto *buffer = m_freeList.back();
        m_freeList.pop_back();

        return std::shared_ptr<types::PCMData>(buffer, [this](types::PCMData *b)
                                               { this->release(b); });
    }

    void BufferPool::release(types::PCMData *buffer)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_freeList.push_back(buffer);
    }
} // namespace dtracker::audio::playback