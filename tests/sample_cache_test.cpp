#include <gtest/gtest.h>

#include <dtracker/sample/cache.hpp>
#include <memory>
#include <string>
#include <vector>

using namespace dtracker::sample;
using PCMData = dtracker::audio::types::PCMData;

class CacheTest : public ::testing::Test
{
  protected:
    Cache cache{2}; // LRU capacity = 2

    std::shared_ptr<PCMData> makePCM(float v, size_t size = 4)
    {
        return std::make_shared<PCMData>(size, v);
    }
};

TEST_F(CacheTest, InsertAndRetrieveSameKey)
{
    cache.insert("sample1", makePCM(1.0f), {44100, 16, 2});
    auto retrieved = cache.get("sample1");

    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ((*retrieved)[0], 1.0f);
}

TEST_F(CacheTest, OverwritesExistingKey)
{
    cache.insert("sample1", makePCM(1.0f), {44100, 16, 2});
    cache.insert("sample1", makePCM(2.0f), {44100, 16, 2});

    auto retrieved = cache.get("sample1");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ((*retrieved)[0], 2.0f);
}

TEST_F(CacheTest, EvictsLeastRecentlyUsed)
{
    cache.insert("a", makePCM(1.0f), {44100, 16, 2});
    cache.insert("b", makePCM(2.0f), {44100, 16, 2});
    cache.get("a"); // access 'a' to make 'b' LRU
    cache.insert("c", makePCM(3.0f), {44100, 16, 2}); // should evict 'b'

    EXPECT_NE(cache.get("a"), nullptr); // still there
    EXPECT_EQ(cache.get("b"), nullptr); // evicted
    EXPECT_NE(cache.get("c"), nullptr); // added
}

TEST_F(CacheTest, EraseKey)
{
    cache.insert("sample", makePCM(9.9f), {44100, 16, 2});
    EXPECT_TRUE(cache.erase("sample"));
    EXPECT_EQ(cache.get("sample"), nullptr);
}

TEST_F(CacheTest, ContainsKey)
{
    cache.insert("k1", makePCM(5.0f), {44100, 16, 2});
    EXPECT_TRUE(cache.contains("k1"));
    EXPECT_FALSE(cache.contains("k2"));
}

TEST_F(CacheTest, ClearCache)
{
    cache.insert("x", makePCM(0.1f), {44100, 16, 2});
    cache.insert("y", makePCM(0.2f), {44100, 16, 2});
    cache.clear();
    EXPECT_EQ(cache.size(), 0);
    EXPECT_FALSE(cache.contains("x"));
}

TEST_F(CacheTest, RespectsCapacityLimit)
{
    cache.setCapacity(1);
    cache.insert("first", makePCM(1.1f), {44100, 16, 2});
    cache.insert("second", makePCM(2.2f), {44100, 16, 2}); // evicts "first"
    EXPECT_FALSE(cache.contains("first"));
    EXPECT_TRUE(cache.contains("second"));
}
