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
    struct CacheEntry
    {
        std::shared_ptr<const audio::types::PCMData> data;
        audio::types::AudioProperties properties;
        std::list<std::string>::iterator useIt;
    };

    class Cache
    {
      public:
        Cache() = default;
        Cache(size_t capacity);

        // Add PCM data to the cache with a unique key
        bool insert(const std::string &key, audio::types::PCMData data,
                    audio::types::AudioProperties properties);

        std::shared_ptr<const audio::types::PCMData>
        get(const std::string &key);

        bool erase(const std::string &key);

        bool contains(const std::string &key) const;

        void setCapacity(size_t capacity);

        size_t capacity() const;

        size_t size() const;

        void clear();

        std::optional<CacheEntry> peek(const std::string &key) const;

      private:
        void evictToCapacity();

        std::unordered_map<std::string, CacheEntry> m_cache;
        mutable std::shared_mutex m_mutex;
        size_t m_capacity{0};
        std::list<std::string> m_useOrder;
    };
} // namespace dtracker::sample