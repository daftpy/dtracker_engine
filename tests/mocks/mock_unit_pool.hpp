#pragma once

#include <gtest/gtest.h>

#include "mock_playback_unit.hpp" // We'll return mock units for testing
#include <dtracker/audio/playback/unit_pool.hpp>
#include <memory>
#include <vector>

// A mock for the UnitPool. Instead of managing a real pool, we can program
// it to return specific mock units on demand for testing purposes.
class MockUnitPool : public dtracker::audio::playback::UnitPool
{
  public:
    // We call the base constructor with a dummy size of 1.
    // It doesn't matter, because we will override its acquire() method.
    MockUnitPool() : dtracker::audio::playback::UnitPool(1) {}

    int acquireCallCount = 0;

    // Override the acquire method to return a pre-programmed unit.
    dtracker::audio::playback::UnitPool::PooledUnitPtr acquire() override
    {
        // Increment our counter every time this is called.
        acquireCallCount++;

        if (m_unitsToReturn.empty())
        {
            // This makes the test fail loudly if acquire() is called
            // more times than we expected.
            ADD_FAILURE() << "MockUnitPool::acquire() called but no units were "
                             "set to be returned.";
            return nullptr;
        }

        // Get the next unit from our queue and return it.
        auto unit = std::move(m_unitsToReturn.back());
        m_unitsToReturn.pop_back();
        return unit;
    }

    // This is a helper method for our tests to queue up a unit that
    // the mock should return when acquire() is called.
    void queueUnit(dtracker::audio::playback::UnitPool::PooledUnitPtr unit)
    {
        m_unitsToReturn.push_back(std::move(unit));
    }

  private:
    std::vector<dtracker::audio::playback::UnitPool::PooledUnitPtr>
        m_unitsToReturn;
};