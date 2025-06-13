#include <dtracker/sample/cache.hpp>

namespace dtracker::sample
{
    Cache::Cache(size_t capacity) : m_capacity(capacity) {}

    bool Cache::insert(const std::string &key,
                       std::shared_ptr<const audio::types::PCMData> data,
                       audio::types::AudioProperties properties)
    {
        // Acquire a unique lock for the entire write operation.
        std::unique_lock lock(m_mutex);

        auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            // Update existing entry
            it->second.data = std::move(data);

            // Move to the front of the usage list to mark as most recently
            // used.
            m_useOrder.erase(it->second.useIt);
            m_useOrder.push_front(key);
            it->second.useIt = m_useOrder.begin();
        }
        else
        {
            // Create new entry
            m_useOrder.push_front(key);
            m_cache[key] = {
                std::move(data), properties,
                m_useOrder
                    .begin() // Iterator points to the front of the LRU list.
            };
        }

        // Remove the least recently used items if over capacity.
        evictToCapacity();
        return true;
    }

    std::shared_ptr<const audio::types::PCMData>
    Cache::get(const std::string &key)
    {
        // Acquire a unique lock because we are modifying the LRU list.
        std::unique_lock lock(m_mutex);

        auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            // Move the accessed item to the front of the usage list.
            m_useOrder.erase(it->second.useIt);
            m_useOrder.push_front(key);
            it->second.useIt = m_useOrder.begin();

            return it->second.data;
        }
        return nullptr;
    }

    bool Cache::erase(const std::string &key)
    {
        std::unique_lock lock(m_mutex);

        auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            m_useOrder.erase(it->second.useIt);
            m_cache.erase(it);
            return true;
        }
        return false;
    }

    // A read-only operation.
    bool Cache::contains(const std::string &key) const
    {
        // Use a shared lock for concurrent read access.
        std::shared_lock lock(m_mutex);
        return m_cache.count(key) > 0;
    }

    void Cache::setCapacity(size_t capacity)
    {
        std::unique_lock lock(m_mutex);
        m_capacity = capacity;
        evictToCapacity();
    }

    size_t Cache::capacity() const
    {
        std::shared_lock lock(m_mutex);
        return m_capacity;
    }

    size_t Cache::size() const
    {
        std::shared_lock lock(m_mutex);
        return m_cache.size();
    }

    void Cache::clear()
    {
        std::unique_lock lock(m_mutex);
        m_cache.clear();
        m_useOrder.clear();
    }

    // Internal helper; must be called from within a unique_lock.
    void Cache::evictToCapacity()
    {
        while (m_capacity > 0 && m_cache.size() > m_capacity)
        {
            // Get the key of the least recently used item (from the back).
            const std::string &lruKey = m_useOrder.back();
            m_cache.erase(lruKey);
            m_useOrder.pop_back();
        }
    }

    // A read-only operation that does not affect LRU order.
    std::optional<types::CacheEntry> Cache::peek(const std::string &key) const
    {
        // Use a shared lock for concurrent read access.
        std::shared_lock lock(m_mutex);

        auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            return it->second;
        }
        return std::nullopt;
    }
} // namespace dtracker::sample