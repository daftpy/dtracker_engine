#pragma once
#include <atomic>
#include <dtracker/audio/types.hpp>
#include <dtracker/sample/cache.hpp>
#include <dtracker/sample/types.hpp>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace dtracker::sample
{
    // Manages the sample workflow, including a temporary data cache and a
    // permanent registry of sample instances. This class is thread-safe.
    class Manager
    {
      public:
        Manager() = default;

        // Caches raw PCM data, returning a shared handle to the cached data.
        std::shared_ptr<const audio::types::PCMData>
        cacheSample(const std::string &sampleLoc,
                    std::shared_ptr<const audio::types::PCMData> pcmData,
                    const types::SampleMetadata &metaData);

        // Caches data AND creates a permanent sample instance with a new
        // unique ID.
        int addSample(const std::string &sampleLoc,
                      std::shared_ptr<const audio::types::PCMData> pcmData,
                      const types::SampleMetadata &metaData);

        // Creates a permanent sample instance from already-cached data.
        int addSample(const std::string &sampleLoc);

        // Retrieves a full sample descriptor (data + metadata) for a given ID.
        std::optional<types::SampleDescriptor> getSample(int id);

        // Removes a sample instance from the registry and its data from the
        // cache.
        bool removeSample(int id);

        // Returns all currently registered sample instance IDs.
        std::vector<int> getAllSampleIds() const;

        // Checks if a sample's data exists in the LRU cache.
        bool contains(const std::string &path) const;

        // Retrieves a cache entry without affecting its position in the LRU
        // list.
        std::optional<types::CacheEntry> peekCache(const std::string &path);

      private:
        // Protects access to the sample registry.
        mutable std::shared_mutex m_registryMutex;

        // Thread-safe generator for unique sample IDs.
        std::atomic<int> m_nextId{0};

        Cache m_cache; // An LRU cache for raw PCM data, keyed by path.

        // Permanent registry of sample instances.
        std::unordered_map<int, types::SampleEntry> m_sampleRegistry;
    };
} // namespace dtracker::sample