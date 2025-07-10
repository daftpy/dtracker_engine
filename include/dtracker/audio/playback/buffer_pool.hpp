#pragma once
#include <dtracker/audio/types.hpp> // For PCMData
#include <memory>
#include <mutex>
#include <vector>

namespace dtracker::audio::playback
{
    /// A thread-safe object pool for recycling fixed-size audio buffers.
    /// This is used to pass audio data from the real-time thread to the GUI
    /// thread without dynamic memory allocation.
    class BufferPool
    {
      public:
        using PooledBufferPtr = std::shared_ptr<types::PCMData>;

        /// Creates a pool of audio buffers.
        /// @param numBuffers The number of buffers to pre-allocate.
        /// @param bufferSize The size (in float samples) of each individual
        /// buffer.
        explicit BufferPool(size_t numBuffers, size_t bufferSize);

        /// Acquires an audio buffer from the pool.
        PooledBufferPtr acquire();

      private:
        friend struct BufferDeleter;
        void release(types::PCMData *buffer);

        std::mutex m_mutex;
        size_t m_bufferSize; // We need to store the size for creating buffers.

        // The pool holds the actual buffer objects.
        std::vector<types::PCMData> m_pool;

        // The free list holds pointers to available buffers.
        std::vector<types::PCMData *> m_freeList;
    };
} // namespace dtracker::audio::playback