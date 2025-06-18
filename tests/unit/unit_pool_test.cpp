#include <gtest/gtest.h>

#include <dtracker/audio/playback/unit_pool.hpp>
#include <thread>
#include <vector>

// Verifies that the pool is initialized with the correct number of objects.
TEST(UnitPool, IsCorrectlySized)
{
    const size_t pool_size = 10;
    dtracker::audio::playback::UnitPool pool(pool_size);

    std::vector<dtracker::audio::playback::UnitPool::PooledUnitPtr>
        acquired_units;

    // Acquire all available objects.
    for (size_t i = 0; i < pool_size; ++i)
    {
        auto unit_ptr = pool.acquire();
        // We should successfully acquire an object each time.
        ASSERT_NE(unit_ptr, nullptr) << "Pool ran out of objects prematurely.";
        acquired_units.push_back(std::move(unit_ptr));
    }

    // Try to acquire one more object than the pool's capacity.
    auto extra_unit_ptr = pool.acquire();

    // The pool should now be empty and return nullptr.
    EXPECT_EQ(extra_unit_ptr, nullptr)
        << "Pool should be exhausted and return nullptr.";
}

// This test verifies that the custom deleter works and returns an object to the
// pool automatically.
TEST(UnitPool, ObjectIsAutomaticallyReleased)
{
    // Create a pool with only one object.
    dtracker::audio::playback::UnitPool pool(1);

    // Acquire the single object.
    auto ptr1 = pool.acquire();
    ASSERT_NE(ptr1, nullptr);

    // At this point, the pool should be empty.
    ASSERT_EQ(pool.acquire(), nullptr);

    // Now, let ptr1 go out of scope by resetting it. This will trigger the
    // custom deleter, which should return the object to the pool.
    ptr1 = nullptr;

    // Try to acquire an object again.
    auto ptr2 = pool.acquire();

    // We should successfully get an object back, proving it was released.
    EXPECT_NE(ptr2, nullptr);
}

// Verifies that the pool is safe to use from multiple threads concurrently.
TEST(UnitPool, IsThreadSafe)
{
    const int pool_size = 50;
    dtracker::audio::playback::UnitPool pool(pool_size);
    std::vector<std::thread> threads;
    const int num_threads = 8;
    const int iterations_per_thread = 100;

    // Create threads that acquire and release objects repeatedly.
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(
            [&pool, iterations_per_thread]()
            {
                for (int j = 0; j < iterations_per_thread; ++j)
                {
                    // Acquire an object.
                    auto unit_ptr = pool.acquire();
                    // When the unit goes out of scope, the object is
                    // automatically returned to the pool.
                }
            });
    }

    // Wait for all threads to finish their work.
    for (auto &t : threads)
    {
        t.join();
    }

    // The pool should be full again.
    std::vector<dtracker::audio::playback::UnitPool::PooledUnitPtr>
        final_acquire;
    final_acquire.reserve(pool_size + 1);

    // Drain the pool completely.
    while (auto ptr = pool.acquire())
    {
        final_acquire.push_back(std::move(ptr));
    }

    // We should acquire exactly the number of objects the pool started
    // with. This proves no objects were lost or duplicated during the test.
    EXPECT_EQ(final_acquire.size(), pool_size);
}