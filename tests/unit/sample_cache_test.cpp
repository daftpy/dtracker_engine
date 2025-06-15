#include <gtest/gtest.h>

#include <dtracker/sample/cache.hpp>
#include <memory>
#include <string>
#include <vector>

using namespace dtracker::sample;
using PCMData = dtracker::audio::types::PCMData;

// Test fixture for the LRU Cache.
class CacheTest : public ::testing::Test
{
  protected:
    // Creates a cache with a small capacity for easier testing of eviction
    // logic.
    Cache cache{2}; // LRU capacity = 2

    // Helper to create dummy PCM data for tests.
    std::shared_ptr<const PCMData> makePCM(float v, size_t size = 4)
    {
        return std::make_shared<const PCMData>(size, v);
    }
};

// Verifies that a newly inserted item can be retrieved.
TEST_F(CacheTest, InsertAndRetrieveSameKey)
{
    cache.insert("sample1", makePCM(1.0f), {44100, 16, 2});
    auto retrieved = cache.get("sample1");

    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ((*retrieved)[0], 1.0f);
}

// Verifies that inserting an existing key overwrites the old data.
TEST_F(CacheTest, OverwritesExistingKey)
{
    cache.insert("sample1", makePCM(1.0f), {44100, 16, 2});
    cache.insert("sample1", makePCM(2.0f),
                 {44100, 16, 2}); // Re-insert with new data.

    auto retrieved = cache.get("sample1");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ((*retrieved)[0], 2.0f);
}

// Verifies that the least recently used item is evicted when capacity is
// reached.
TEST_F(CacheTest, EvictsLeastRecentlyUsed)
{
    cache.insert("a", makePCM(1.0f), {44100, 16, 2});
    cache.insert("b", makePCM(2.0f), {44100, 16, 2});
    cache.get("a"); // Access 'a' to make 'b' the least recently used.
    cache.insert("c", makePCM(3.0f), {44100, 16, 2}); // This should evict 'b'.

    EXPECT_NE(cache.get("a"), nullptr); // Should still be present.
    EXPECT_EQ(cache.get("b"), nullptr); // Should be evicted.
    EXPECT_NE(cache.get("c"), nullptr); // The new item should be present.
}

// Verifies that `peek` retrieves data without updating the LRU order.
TEST_F(CacheTest, PeekDoesNotUpdateLru)
{
    cache.insert("a", makePCM(1.0f), {44100, 16, 2});
    cache.insert("b", makePCM(2.0f),
                 {44100, 16, 2}); // 'a' is now the least recently used.
    cache.peek("a"); // Peek at 'a'; this should NOT change its LRU status.
    cache.insert("c", makePCM(3.0f), {44100, 16, 2}); // This should evict 'a'.

    EXPECT_EQ(cache.get("a"), nullptr); // Should be evicted.
    EXPECT_NE(cache.get("b"), nullptr);
    EXPECT_NE(cache.get("c"), nullptr);
}

// Verifies that `erase` successfully removes an item.
TEST_F(CacheTest, EraseKey)
{
    cache.insert("sample", makePCM(9.9f), {44100, 16, 2});
    EXPECT_TRUE(cache.erase("sample"));
    EXPECT_EQ(cache.get("sample"), nullptr); // Verify it's gone.
}

// Verifies that `contains` correctly reports the presence of a key.
TEST_F(CacheTest, ContainsKey)
{
    cache.insert("k1", makePCM(5.0f), {44100, 16, 2});
    EXPECT_TRUE(cache.contains("k1"));
    EXPECT_FALSE(cache.contains("k2"));
}

// Verifies that `clear` removes all items from the cache.
TEST_F(CacheTest, ClearCache)
{
    cache.insert("x", makePCM(0.1f), {44100, 16, 2});
    cache.insert("y", makePCM(0.2f), {44100, 16, 2});
    cache.clear();

    EXPECT_EQ(cache.size(), 0);
    EXPECT_FALSE(cache.contains("x"));
}

// Verifies that the cache does not exceed a new, smaller capacity.
TEST_F(CacheTest, RespectsCapacityLimit)
{
    cache.setCapacity(1);
    cache.insert("first", makePCM(1.1f), {44100, 16, 2});
    cache.insert("second", makePCM(2.2f),
                 {44100, 16, 2}); // This should evict "first".

    EXPECT_FALSE(cache.contains("first"));
    EXPECT_TRUE(cache.contains("second"));
}

// Verifies that audio properties are stored and retrieved correctly.
TEST_F(CacheTest, StoresAndRetrievesProperties)
{
    dtracker::audio::types::AudioProperties props = {48000, 24, 1};
    cache.insert("mono_sample", makePCM(0.5f), props);

    auto retrievedEntry = cache.peek("mono_sample");
    ASSERT_TRUE(retrievedEntry.has_value());
    EXPECT_EQ(retrievedEntry->properties.sampleRate, 48000);
    EXPECT_EQ(retrievedEntry->properties.bitDepth, 24);
    EXPECT_EQ(retrievedEntry->properties.numChannels, 1);
}
