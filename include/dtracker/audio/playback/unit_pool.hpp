#pragma once
#include <dtracker/audio/playback/sample_playback_unit.hpp>
#include <functional>
#include <memory>
#include <mutex>

namespace dtracker::audio::playback
{
    class UnitPool
    {
      public:
        /// A unique_ptr that automatically returns its object to the pool
        /// when it goes out of scope.
        using PooledUnitPtr =
            std::unique_ptr<SamplePlaybackUnit,
                            std::function<void(SamplePlaybackUnit *)>>;

        /// Creates a pool with a fixed number of pre-allocated units.
        /// @param size The number of SamplePlaybackUnits to create up front.
        explicit UnitPool(size_t size);

        /// Acquires a playback unit from the pool.
        /// @return A smart pointer to a recycled unit, or nullptr if the pool
        /// is empty.
        PooledUnitPtr acquire();

      private:
        /// Returns a unit to the pool's free list.
        /// This is called automatically by the PooledUnitPtr's custom deleter.
        void release(SamplePlaybackUnit *unit);

        std::mutex m_mutex;
        // This vector holds the actual memory for all the objects.
        // They are constructed once and live for the lifetime of the pool.
        std::vector<SamplePlaybackUnit> m_pool;

        // This list contains raw pointers to the objects in the pool that are
        // currently available to be used.
        std::vector<SamplePlaybackUnit *> m_freeList;
    };
} // namespace dtracker::audio::playback