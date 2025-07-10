#include <gtest/gtest.h>

#include <dtracker/audio/playback/buffer_pool.hpp> // The class we are testing
#include <memory>
#include <thread>
#include <vector>

using namespace dtracker::audio;
using namespace dtracker::audio::playback;

// Verifies that the pool is initialized with the correct number of buffers.
TEST(BufferPool, IsCorrectlySized)
{
    const size_t num_buffers = 10;
    const size_t buffer_size = 256;
    BufferPool pool(num_buffers, buffer_size);

    std::vector<BufferPool::PooledBufferPtr> acquired_buffers;

    // Acquire all available buffers.
    for (size_t i = 0; i < num_buffers; ++i)
    {
        auto buffer_ptr = pool.acquire();
        // We should successfully acquire a buffer each time.
        ASSERT_NE(buffer_ptr, nullptr)
            << "Pool ran out of buffers prematurely.";
        // Verify the buffer has the correct size.
        ASSERT_EQ(buffer_ptr->size(), buffer_size);
        acquired_buffers.push_back(std::move(buffer_ptr));
    }

    // Try to acquire one more buffer than the pool's capacity.
    auto extra_buffer_ptr = pool.acquire();

    // The pool should now be empty and return nullptr.
    EXPECT_EQ(extra_buffer_ptr, nullptr)
        << "Pool should be exhausted and return nullptr.";
}

// Verifies that the custom deleter works and returns a buffer to the pool
// automatically.
TEST(BufferPool, ObjectIsAutomaticallyReleased)
{
    // Create a pool with only one buffer.
    BufferPool pool(1, 256);

    // Acquire the single buffer.
    auto ptr1 = pool.acquire();
    ASSERT_NE(ptr1, nullptr);

    // At this point, the pool should be empty.
    ASSERT_EQ(pool.acquire(), nullptr);

    // Now, let ptr1 go out of scope by resetting it. This will trigger
    // the custom deleter, which should return the buffer to the pool.
    ptr1 = nullptr;

    // Try to acquire a buffer again.
    auto ptr2 = pool.acquire();

    // We should successfully get a buffer back, proving it was
    // released.
    EXPECT_NE(ptr2, nullptr);
}

// // Verifies that the pool is safe to use from multiple threads concurrently.
TEST(BufferPool, IsThreadSafe)
{
    const int num_buffers = 50;
    const int buffer_size = 128;
    BufferPool pool(num_buffers, buffer_size);
    std::vector<std::thread> threads;
    const int num_threads = 8;
    const int iterations_per_thread = 100;

    // Create threads that acquire and release buffers repeatedly.
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(
            [&pool, iterations_per_thread]()
            {
                for (int j = 0; j < iterations_per_thread; ++j)
                {
                    // Acquire a buffer.
                    auto buffer_ptr = pool.acquire();
                    // When buffer_ptr goes out of scope here, the object is
                    // automatically returned to the pool.
                }
            });
    }

    // Wait for all threads to finish their work.
    for (auto &t : threads)
    {
        t.join();
    }

    // --- Verification Step ---
    // Now that all threads are done, the pool should be full again.
    std::vector<BufferPool::PooledBufferPtr> final_acquire;
    final_acquire.reserve(num_buffers + 1);

    // Drain the pool completely.
    while (auto ptr = pool.acquire())
    {
        final_acquire.push_back(std::move(ptr));
    }

    // Assert that we acquired exactly the number of objects the pool started
    // with. This proves no buffers were lost or duplicated during the
    // threaded test.
    EXPECT_EQ(final_acquire.size(), num_buffers);
}