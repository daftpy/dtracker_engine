#include <dtracker/sample/manager.hpp>

namespace dtracker::sample
{
    // Caches the sample data and then retrieves it to update its LRU status.
    std::shared_ptr<const audio::types::PCMData>
    Manager::cacheSample(const std::string &sampleLoc,
                         std::shared_ptr<const audio::types::PCMData> pcmData,
                         const types::SampleMetadata &metaData)
    {
        // Insert (or update) the sample in the cache, moving the data
        // efficiently.
        m_cache.insert(sampleLoc, std::move(pcmData),
                       {metaData.sourceSampleRate, metaData.bitDepth, 2});

        // Get the sample to mark it as most recently used.
        return m_cache.get(sampleLoc);
    }

    // Caches raw data and creates a permanent registered instance in one
    // operation.
    int Manager::addSample(const std::string &sampleLoc,
                           std::shared_ptr<const audio::types::PCMData> pcmData,
                           const types::SampleMetadata &metaData)
    {
        // The cache has its own internal locking, so this call is thread-safe.
        m_cache.insert(sampleLoc, std::move(pcmData),
                       {metaData.sourceSampleRate, metaData.bitDepth, 2});

        // Lock the registry to safely generate an ID and add the new entry.
        std::unique_lock lock(m_registryMutex);
        auto id = m_nextId++;
        m_sampleRegistry[id] = {id, sampleLoc, metaData};
        return id;
    }

    // Creates a registered sample instance from data that is already in the
    // cache.
    int Manager::addSample(const std::string &sampleLoc)
    {
        // Check cache without locking the registry or affecting LRU order.
        auto cacheEntry = m_cache.peek(sampleLoc);
        if (!cacheEntry.has_value())
        {
            return -1; // Fail if source data doesn't exist in the cache.
        }

        // Extract metadata from the cached properties.
        const auto &entry = cacheEntry.value();
        types::SampleMetadata metaData;
        metaData.sourceSampleRate = entry.properties.sampleRate;
        metaData.bitDepth = entry.properties.bitDepth;

        // Lock the registry to safely generate an ID and add the new entry.
        std::unique_lock lock(m_registryMutex);
        auto id = m_nextId++;
        m_sampleRegistry[id] = {id, sampleLoc, metaData};
        return id;
    }

    // Constructs a full SampleDescriptor from a registered ID.
    std::optional<types::SampleDescriptor> Manager::getSample(int id)
    {
        // Use a read-lock, allowing multiple threads to get samples
        // concurrently.
        std::shared_lock lock(m_registryMutex);

        auto it = m_sampleRegistry.find(id);
        if (it != m_sampleRegistry.end())
        {
            const auto &entry = it->second;
            // Get from cache, which updates the LRU order.
            auto pcm = m_cache.get(entry.registryKey);
            if (pcm)
            {
                // Move the retrieved shared_ptr for efficiency.
                return types::SampleDescriptor{entry.id, std::move(pcm),
                                               entry.metaData};
            }
        }
        return std::nullopt;
    }

    // Peeks into the cache without affecting the LRU order.
    std::optional<types::CacheEntry> Manager::peekCache(const std::string &path)
    {
        return m_cache.peek(path);
    }

    // Removes a sample from both the registry and the underlying cache.
    bool Manager::removeSample(int id)
    {
        // Use a write-lock for the entire removal operation.
        std::unique_lock lock(m_registryMutex);
        auto it = m_sampleRegistry.find(id);

        if (it != m_sampleRegistry.end())
        {
            m_cache.erase(it->second.registryKey);
            m_sampleRegistry.erase(it);
            return true;
        }
        return false;
    }

    // Gathers all registered IDs into a vector.
    std::vector<int> Manager::getAllSampleIds() const
    {
        // Use a read-lock as we are not modifying the registry.
        std::shared_lock lock(m_registryMutex);

        std::vector<int> ids;
        ids.reserve(m_sampleRegistry.size());

        for (const auto &[id, entry] : m_sampleRegistry)
        {
            ids.push_back(id);
        }
        return ids;
    }

    // Checks for data presence in the cache.
    bool Manager::contains(const std::string &path) const
    {
        // The cache has its own internal locking.
        return m_cache.contains(path);
    }
} // namespace dtracker::sample