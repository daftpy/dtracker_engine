#pragma once

#include <dtracker/audio/types.hpp>
#include <dtracker/sample/types.hpp>
#include <list>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace dtracker::sample
{
    // A thread-safe, capacity-constrained, Least Recently Used (LRU) cache.
    // Stores shared pointers to audio data, keyed by a string path.
    class Cache
    {
      public:
        Cache() = default;
        explicit Cache(size_t capacity);

        // Inserts or updates an entry. Marks the item as most recently used.
        bool insert(const std::string &key,
                    std::shared_ptr<const audio::types::PCMData> data,
                    audio::types::AudioProperties properties);

        // Retrieves an entry and marks it as most recently used.
        std::shared_ptr<const audio::types::PCMData>
        get(const std::string &key);

        // Removes an entry from the cache.
        bool erase(const std::string &key);

        // Checks for an entry's existence without changing its LRU status.
        bool contains(const std::string &key) const;

        // Sets the maximum number of items the cache can hold, evicting if
        // needed.
        void setCapacity(size_t capacity);

        // Returns the maximum number of items the cache can hold.
        size_t capacity() const;

        // Returns the current number of items in the cache.
        size_t size() const;

        // Clears all entries from the cache.
        void clear();

        // Retrieves an entry's data without changing its LRU status.
        std::optional<types::CacheEntry> peek(const std::string &key) const;

      private:
        // Private helper to remove the least recently used items until at
        // capacity.
        void evictToCapacity();

        // The main storage for cache entries.
        std::unordered_map<std::string, types::CacheEntry> m_cache;
        // A mutex to protect all access to the cache data structures.
        mutable std::shared_mutex m_mutex;
        // The maximum number of entries the cache will store. 0 means
        // unlimited.
        size_t m_capacity{0};
        // A list of keys to track usage order for the LRU policy.
        std::list<std::string> m_useOrder;
    };
} // namespace dtracker::sample