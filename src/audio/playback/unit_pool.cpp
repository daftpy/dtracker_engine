#include <dtracker/audio/playback/unit_pool.hpp>
#include <stdexcept>

namespace dtracker::audio::playback
{
    // Creates a pool with a fixed number of pre-allocated SamplePlaybackUnits.
    UnitPool::UnitPool(size_t size)
    {
        if (size == 0)
        {
            throw std::invalid_argument("Object pool size cannot be zero.");
        }

        // 1. Allocate the memory for all the objects at once.
        //    This calls the default constructor for each SamplePlaybackUnit.
        m_pool.resize(size);

        // 2. Pre-allocate the memory for our list of free pointers to
        //    avoid reallocations later, which is important for performance.
        m_freeList.reserve(size);

        // 3. Populate the free list with pointers to every object in our pool.
        //    Initially, all objects are available.
        for (auto &unit : m_pool)
        {
            m_freeList.push_back(&unit);
        }
    }

    // Acquires a playback unit from the pool.
    UnitPool::PooledUnitPtr UnitPool::acquire()
    {
        // Lock the mutex to protect the free list during modification.
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_freeList.empty())
        {
            // All objects are currently in use. This is a "voice stealing"
            // scenario that a real DAW would handle, but for now, we fail
            // gracefully.
            return nullptr;
        }

        // 1. Get a pointer to a free object from the end of the list.
        SamplePlaybackUnit *unit = m_freeList.back();
        m_freeList.pop_back();

        // 2. Create the custom deleter lambda. It captures a pointer to this
        //    pool (`this`) so it knows where to return the object.
        auto deleter = [this](SamplePlaybackUnit *returnedUnit)
        { this->release(returnedUnit); };

        // 3. Return the raw pointer wrapped in a unique_ptr that uses our
        //    custom deleter instead of calling `delete`.
        return PooledUnitPtr(unit, deleter);
    }

    // Returns a unit to the pool's free list.
    void UnitPool::release(SamplePlaybackUnit *unit)
    {
        // Lock the mutex to protect the free list during modification.
        std::lock_guard<std::mutex> lock(m_mutex);

        // Add the pointer back to the list, making it available for the next
        // acquire().
        m_freeList.push_back(unit);
    }

} // namespace dtracker::audio::playback