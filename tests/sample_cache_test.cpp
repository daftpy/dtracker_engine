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

    PCMData makePCM(float v, size_t size = 4)
    {
        return PCMData(size, v);
    }
};

TEST_F(CacheTest, InsertAndRetrieveSameKey)
{
    cache.insert("sample1", makePCM(1.0f));
    auto retrieved = cache.get("sample1");

    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ((*retrieved)[0], 1.0f);
}

TEST_F(CacheTest, OverwritesExistingKey)
{
    cache.insert("sample1", makePCM(1.0f));
    cache.insert("sample1", makePCM(2.0f));

    auto retrieved = cache.get("sample1");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ((*retrieved)[0], 2.0f);
}

TEST_F(CacheTest, EvictsLeastRecentlyUsed)
{
    cache.insert("a", makePCM(1.0f));
    cache.insert("b", makePCM(2.0f));
    cache.get("a");                   // access 'a' to make 'b' LRU
    cache.insert("c", makePCM(3.0f)); // should evict 'b'

    EXPECT_NE(cache.get("a"), nullptr); // still there
    EXPECT_EQ(cache.get("b"), nullptr); // evicted
    EXPECT_NE(cache.get("c"), nullptr); // added
}

TEST_F(CacheTest, EraseKey)
{
    cache.insert("sample", makePCM(9.9f));
    EXPECT_TRUE(cache.erase("sample"));
    EXPECT_EQ(cache.get("sample"), nullptr);
}

TEST_F(CacheTest, ContainsKey)
{
    cache.insert("k1", makePCM(5.0f));
    EXPECT_TRUE(cache.contains("k1"));
    EXPECT_FALSE(cache.contains("k2"));
}

TEST_F(CacheTest, ClearCache)
{
    cache.insert("x", makePCM(0.1f));
    cache.insert("y", makePCM(0.2f));
    cache.clear();
    EXPECT_EQ(cache.size(), 0);
    EXPECT_FALSE(cache.contains("x"));
}

TEST_F(CacheTest, RespectsCapacityLimit)
{
    cache.setCapacity(1);
    cache.insert("first", makePCM(1.1f));
    cache.insert("second", makePCM(2.2f)); // evicts "first"
    EXPECT_FALSE(cache.contains("first"));
    EXPECT_TRUE(cache.contains("second"));
}
