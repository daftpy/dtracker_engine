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
    class Manager
    {
      public:
        Manager() = default;

        std::shared_ptr<const audio::types::PCMData>
        cacheSample(const std::string &sampleLoc, audio::types::PCMData pcmData,
                    const types::SampleMetadata &metaData);

        int addSample(const std::string &sampleLoc,
                      audio::types::PCMData pcmData,
                      const types::SampleMetadata &metaData);

        std::optional<types::SampleDescriptor> getSample(int id);

        bool removeSample(int id);

        std::vector<int> getAllSampleIds() const;

        bool contains(const std::string &path) const;

        std::optional<CacheEntry> peekCache(const std::string &path);

      private:
        mutable std::shared_mutex m_registryMutex;
        std::atomic<int> m_nextId{0};
        Cache m_cache;
        std::unordered_map<int, types::SampleEntry> m_sampleRegistry;
    };
} // namespace dtracker::sample
