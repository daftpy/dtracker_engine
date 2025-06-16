#include <gtest/gtest.h>

#include <dtracker/audio/playback/unit_pool.hpp> // The class we are testing
#include <thread>
#include <vector>


// For this test, we don't need a full SamplePlaybackUnit.
// A simple struct is enough to test the pool's mechanics.
struct DummyUnit
{
    int id = 0;
};

// We need to specialize the UnitPool for our DummyUnit for these tests.
// This is a bit of a workaround because we made the pool non-template.
// In a real-world scenario, the pool would likely be a template.
using DummyUnitPool =
    dtracker::audio::playback::UnitPool; // Let's assume UnitPool can hold
                                         // DummyUnit-like objects For the real
                                         // code, this would be a template. For
                                         // this example, we will just imagine
                                         // the pool works with DummyUnit.
// To make this compile, you would temporarily change your UnitPool to be a pool
// of DummyUnit. For the sake of this example, we will assume it is. The logic
// is the same.

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

    // Act: Try to acquire one more object than the pool's capacity.
    auto extra_unit_ptr = pool.acquire();

    // Assert: The pool should now be empty and return nullptr.
    EXPECT_EQ(extra_unit_ptr, nullptr)
        << "Pool should be exhausted and return nullptr.";
}

// This is the most important test. It verifies that the custom deleter
// works and returns an object to the pool automatically.
TEST(UnitPool, ObjectIsAutomaticallyReleased)
{
    // Create a pool with only one object.
    dtracker::audio::playback::UnitPool pool(1);

    // Acquire the single object.
    auto ptr1 = pool.acquire();
    ASSERT_NE(ptr1, nullptr);

    // At this point, the pool should be empty.
    ASSERT_EQ(pool.acquire(), nullptr);

    // Now, let ptr1 go out of scope by resetting it. This will trigger
    // the custom deleter, which should return the object to the pool.
    ptr1 = nullptr;

    // Act: Try to acquire an object again.
    auto ptr2 = pool.acquire();

    // Assert: We should successfully get an object back, proving it was
    // released.
    EXPECT_NE(ptr2, nullptr);
}

// Verifies that the pool is safe to use from multiple threads concurrently.
TEST(UnitPool, IsThreadSafe)
{
    // Use a larger pool for the stress test.
    const int pool_size = 50;
    dtracker::audio::playback::UnitPool pool(pool_size);
    std::vector<std::thread> threads;
    const int num_threads = 8;

    // Each thread will acquire and release objects many times.
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(
            [&pool]()
            {
                for (int j = 0; j < 100; ++j)
                {
                    // Acquire an object. The unique_ptr owns it.
                    auto unit_ptr = pool.acquire();
                    if (unit_ptr)
                    {
                        // Do some "work"
                        std::this_thread::sleep_for(
                            std::chrono::microseconds(10));
                    }
                    // When unit_ptr goes out of scope at the end of this loop,
                    // the object is automatically returned to the pool.
                }
            });
    }

    // Wait for all threads to finish.
    for (auto &t : threads)
    {
        t.join();
    }

    // To verify, we'll try to acquire all objects. If the pool is not corrupt,
    // we should be able to get exactly pool_size objects back.
    for (int i = 0; i < pool_size; ++i)
    {
        EXPECT_NE(pool.acquire(), nullptr)
            << "Pool lost objects during threaded access.";
    }
    EXPECT_EQ(pool.acquire(), nullptr)
        << "Pool has extra objects after threaded access.";
}