#include <dtracker/sample/cache.hpp>

namespace dtracker::sample
{
    Cache::Cache(size_t capacity) : m_capacity(capacity) {}

    bool Cache::insert(const std::string &key, audio::types::PCMData data)
    {
        // Lock for writes to update LRU pos
        std::unique_lock lock(m_mutex);

        // If it already exists
        auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            // Update the data
            it->second.data =
                std::make_shared<audio::types::PCMData>(std::move(data));

            // Update LRU
            m_useOrder.erase(it->second.useIt);
            m_useOrder.push_front(key);

            // Update iter member
            it->second.useIt = m_useOrder.begin();
        }
        // otherise, create the new entry
        else
        {
            m_useOrder.push_front(key);
            m_cache[key] = {
                std::make_shared<audio::types::PCMData>(std::move(data)),
                m_useOrder.begin() // Front of the LRU
            };
        }
        evictToCapacity();
        return true;
    }

    std::shared_ptr<const audio::types::PCMData>
    Cache::get(const std::string &key)
    {
        // Lock for writes to update LRU
        std::unique_lock lock(m_mutex);

        // If it exists
        auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            // Update the LRU pos
            m_useOrder.erase(it->second.useIt);
            m_useOrder.push_front(key);

            // Return the data
            it->second.useIt = m_useOrder.begin();
            return it->second.data;
        }
        return nullptr;
    }

    bool Cache::erase(const std::string &key)
    {
        std::unique_lock lock(m_mutex);

        // If the key exists
        auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            // Erase it from the LRU and Cache
            m_useOrder.erase(it->second.useIt);
            m_cache.erase(it);
            return true;
        }
        return false;
    }

    bool Cache::contains(const std::string &key) const
    {
        std::shared_lock lock(m_mutex);
        return m_cache.find(key) != m_cache.end();
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

    void Cache::evictToCapacity()
    {
        while (m_capacity > 0 && m_cache.size() > m_capacity)
        {
            const std::string &lruKey = m_useOrder.back();
            m_cache.erase(lruKey);
            m_useOrder.pop_back();
        }
    }
} // namespace dtracker::sample