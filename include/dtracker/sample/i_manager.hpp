#pragma once
#include <dtracker/audio/types.hpp>
#include <dtracker/sample/types.hpp>
#include <memory>
#include <optional>
#include <string>

namespace dtracker::sample
{
    class IManager
    {
      public:
        virtual ~IManager() = default;

        // Caches raw PCM data, returning a shared handle to the cached data.
        virtual std::shared_ptr<const audio::types::PCMData>
        cacheSample(const std::string &sampleLoc,
                    std::shared_ptr<const audio::types::PCMData> pcmData,
                    const types::SampleMetadata &metaData) = 0;

        // Caches data AND creates a permanent sample instance with a new
        // unique ID.
        virtual int
        addSample(const std::string &sampleLoc,
                  std::shared_ptr<const audio::types::PCMData> pcmData,
                  const types::SampleMetadata &metaData) = 0;

        // Creates a permanent sample instance from already-cached data.
        virtual int addSample(const std::string &sampleLoc) = 0;

        // Retrieves a full sample descriptor (data + metadata) for a given ID.
        virtual std::optional<types::SampleDescriptor> getSample(int id) = 0;

        virtual bool removeSample(int id) = 0;

        virtual std::vector<int> getAllSampleIds() const = 0;

        // Checks if a sample's data exists in the LRU cache.
        virtual bool contains(const std::string &path) const = 0;

        // Retrieves a cache entry without affecting its position in the LRU
        // list.
        virtual std::optional<types::CacheEntry>
        peekCache(const std::string &path) = 0;
    };
} // namespace dtracker::sample