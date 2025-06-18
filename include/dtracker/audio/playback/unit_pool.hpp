#pragma once

#include "sample_playback_unit.hpp"
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace dtracker::audio::playback
{
    /// A thread-safe object pool for recycling SamplePlaybackUnits.
    /// This avoids memory allocation on the real-time audio thread.
    class UnitPool
    {
      public:
        /// A unique_ptr that automatically returns its object to the pool
        /// when its scope ends, thanks to a custom deleter.
        using PooledUnitPtr = std::shared_ptr<SamplePlaybackUnit>;

        /// Creates a pool with a fixed number of pre-allocated units.
        /// @param size The number of SamplePlaybackUnits to create up front.
        explicit UnitPool(size_t size);

        /// Acquires a playback unit from the pool.
        /// @return A smart pointer to a recycled unit, or nullptr if the pool
        /// is exhausted.
        PooledUnitPtr acquire();

      private:
        /// Returns a unit to the pool's free list.
        /// This is called automatically by the PooledUnitPtr's custom deleter.
        void release(SamplePlaybackUnit *unit);

        /// A mutex to protect access to the free list from multiple threads.
        std::mutex m_mutex;

        /// Holds the actual, pre-allocated object memory. These objects live
        /// for the entire lifetime of the pool.
        std::vector<SamplePlaybackUnit> m_pool;

        /// A list of raw pointers to the objects in m_pool that are currently
        /// available to be used.
        std::vector<SamplePlaybackUnit *> m_freeList;
    };
} // namespace dtracker::audio::playback