#include <dtracker/audio/playback/unit_pool.hpp>
#include <dtracker/sample/types.hpp>
#include <stdexcept>

namespace dtracker::audio::playback
{
    // Creates a pool with a fixed number of pre-allocated SamplePlaybackUnits.
    UnitPool::UnitPool(size_t size)
    {
        if (size == 0)
            throw std::invalid_argument("Object pool size cannot be zero.");

        // Pre-allocate vector memory once to avoid reallocations during setup.
        m_pool.reserve(size);
        m_freeList.reserve(size);

        // Create all objects up front and populate the free list.
        for (size_t i = 0; i < size; ++i)
        {
            // Create a "blank" unit in place.
            m_pool.emplace_back(sample::types::SampleDescriptor{});
            // Add a pointer to the newly created unit to the free list.
            m_freeList.push_back(&m_pool[i]);
        }
    }

    // Acquires an available unit from the pool.
    UnitPool::PooledUnitPtr UnitPool::acquire()
    {
        // Lock to ensure thread-safe modification of the free list.
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_freeList.empty())
            return nullptr; // Return null if all objects are in use.

        // Get a pointer from the end of the list.
        SamplePlaybackUnit *unit = m_freeList.back();
        m_freeList.pop_back();

        // Sanity check to help debug potential logic errors.
        if (unit->isCheckedOut)
            throw std::logic_error(
                "Object pool corruption: Double checkout detected!");

        unit->isCheckedOut = true;

        // Return the raw pointer wrapped in a unique_ptr that uses a custom
        // deleter.
        return PooledUnitPtr(unit,
                             [this](SamplePlaybackUnit *u) { release(u); });
    }

    // Returns a unit's pointer back to the free list.
    void UnitPool::release(SamplePlaybackUnit *unit)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Sanity check to guard against releasing the same object twice.
        if (!unit->isCheckedOut)
            throw std::logic_error(
                "Object pool corruption: Double release detected!");

        unit->reset(); // Reset the unit to a clean state for its next use.
        unit->isCheckedOut = false;

        // Add the pointer back to the list, making it available for the next
        // acquire().
        m_freeList.push_back(unit);
    }

} // namespace dtracker::audio::playback