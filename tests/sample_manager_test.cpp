#include <gtest/gtest.h>

#include <algorithm>
#include <dtracker/sample/manager.hpp>
#include <memory>
#include <string>
#include <thread> // Required for concurrency test
#include <vector>


// Verifies that each call to addSample returns a new, unique ID.
TEST(SampleManager, AddReturnsUniqueIds)
{
    dtracker::sample::Manager manager;

    int id1 = manager.addSample(
        "sample1",
        std::make_shared<const dtracker::audio::types::PCMData>(
            dtracker::audio::types::PCMData{0.1f, 0.2f, 0.3f}),
        {44100, 32});

    int id2 = manager.addSample(
        "sample2",
        std::make_shared<const dtracker::audio::types::PCMData>(
            dtracker::audio::types::PCMData{0.4f, 0.5f}),
        {44100, 32});

    EXPECT_NE(id1, id2);
}

// Verifies that getSample returns a nullopt for an ID that doesn't exist.
TEST(SampleManager, GetSampleReturnsNullOnInvalidId)
{
    dtracker::sample::Manager manager;
    auto sample = manager.getSample(999); // 999 was never added.
    EXPECT_EQ(sample, std::nullopt);
}

// Verifies that a removed sample can no longer be retrieved.
TEST(SampleManager, RemoveSampleDeletesIt)
{
    dtracker::sample::Manager manager;
    int id = manager.addSample(
        "sample1",
        std::make_shared<const dtracker::audio::types::PCMData>(
            dtracker::audio::types::PCMData{0.1f, 0.2f, 0.3f}),
        {44100, 32});

    EXPECT_TRUE(manager.removeSample(id));
    EXPECT_FALSE(
        manager.removeSample(id)); // Should fail, as it's already gone.
    EXPECT_EQ(manager.getSample(id), std::nullopt);
}

// Verifies that the list of all IDs correctly reflects additions and removals.
TEST(SampleManager, AllSampleIdsReflectsContents)
{
    dtracker::sample::Manager manager;
    int id1 = manager.addSample(
        "sample1",
        std::make_shared<const dtracker::audio::types::PCMData>(
            dtracker::audio::types::PCMData{0.1f}),
        {44100, 16});
    int id2 = manager.addSample(
        "sample2",
        std::make_shared<const dtracker::audio::types::PCMData>(
            dtracker::audio::types::PCMData{0.2f}),
        {44100, 16});

    std::vector<int> ids = manager.getAllSampleIds();
    EXPECT_EQ(ids.size(), 2);
    // Check that both IDs are present in the returned vector.
    EXPECT_NE(std::find(ids.begin(), ids.end(), id1), ids.end());
    EXPECT_NE(std::find(ids.begin(), ids.end(), id2), ids.end());

    manager.removeSample(id1);
    ids = manager.getAllSampleIds();
    EXPECT_EQ(ids.size(), 1);
    EXPECT_EQ(ids[0], id2);
}

// Verifies the simpler addSample overload fails if the data is not yet cached.
TEST(SampleManager, AddSampleFailsIfNotFoundInCache)
{
    dtracker::sample::Manager manager;
    // Attempt to register a sample that has not been cached first.
    int id = manager.addSample("non_existent_sample");
    EXPECT_EQ(id, -1);
}

// Verifies that multiple instances can be created from the same cached data.
TEST(SampleManager, CanAddMultipleInstancesFromSameSource)
{
    dtracker::sample::Manager manager;
    const std::string samplePath = "shared_sample";

    // 1. Cache the data once using the full addSample overload.
    manager.addSample(samplePath,
                      std::make_shared<const dtracker::audio::types::PCMData>(
                          dtracker::audio::types::PCMData{0.1f}),
                      {44100, 16});

    // 2. Register two separate instances using the simpler overload.
    int id1 = manager.addSample(samplePath);
    int id2 = manager.addSample(samplePath);

    // 3. Check that they are valid (not -1) and have different IDs.
    EXPECT_NE(id1, -1);
    EXPECT_NE(id2, -1);
    EXPECT_NE(id1, id2);

    // 4. Verify both can be retrieved successfully.
    EXPECT_TRUE(manager.getSample(id1).has_value());
    EXPECT_TRUE(manager.getSample(id2).has_value());
}

// Verifies that adding samples is safe from multiple threads.
TEST(SampleManager, IsThreadSafe)
{
    dtracker::sample::Manager manager;
    std::vector<std::thread> threads;
    const int num_threads = 10;
    const int samples_per_thread = 100;

    // Create multiple threads that all add samples concurrently.
    for (int i = 0; i < num_threads; ++i)
    {
        // FIX: Use [=, &manager] to capture outside variables.
        // [=] captures i and samples_per_thread by value.
        // &manager captures the manager by reference.
        threads.emplace_back(
            [=, &manager]()
            {
                for (int j = 0; j < samples_per_thread; ++j)
                {
                    // Create a unique key for each sample to avoid overwrites.
                    std::string key =
                        "sample_" + std::to_string(i) + "_" + std::to_string(j);
                    manager.addSample(
                        key,
                        std::make_shared<const dtracker::audio::types::PCMData>(
                            dtracker::audio::types::PCMData{0.1f}),
                        {44100, 16});
                }
            });
    }

    // Wait for all threads to finish their work.
    for (auto &t : threads)
    {
        t.join();
    }

    // The final size of the registry should be the total number of samples
    // added. If this fails, it indicates a race condition in ID generation or
    // map insertion.
    EXPECT_EQ(manager.getAllSampleIds().size(),
              num_threads * samples_per_thread);
}