#include <dtracker/sample/manager.hpp>

namespace dtracker::sample
{
    std::shared_ptr<const audio::types::PCMData>
    Manager::cacheSample(const std::string &sampleLoc,
                         std::shared_ptr<const audio::types::PCMData> pcmData,
                         const types::SampleMetadata &metaData)
    {
        m_cache.insert(sampleLoc, std::move(pcmData),
                       {metaData.sourceSampleRate, metaData.bitDepth, 2});

        return m_cache.get(sampleLoc);
    }

    int Manager::addSample(const std::string &sampleLoc,
                           std::shared_ptr<const audio::types::PCMData> pcmData,
                           const types::SampleMetadata &metaData)
    {
        m_cache.insert(sampleLoc, std::move(pcmData),
                       {metaData.sourceSampleRate, metaData.bitDepth, 2});

        std::unique_lock lock(m_registryMutex);
        auto id = m_nextId++;

        m_sampleRegistry[id] = {id, sampleLoc, metaData};
        return id;
    }

    std::optional<types::SampleDescriptor> Manager::getSample(int id)
    {
        std::shared_lock lock(m_registryMutex);

        auto it = m_sampleRegistry.find(id);
        if (it != m_sampleRegistry.end())
        {
            const auto &entry = it->second;
            auto pcm = m_cache.get(entry.registryKey);
            if (pcm)
            {
                return types::SampleDescriptor{entry.id, std::move(pcm),
                                               entry.metaData};
            }
        }
        return std::nullopt;
    }

    std::optional<CacheEntry>
    dtracker::sample::Manager::peekCache(const std::string &path)
    {
        return m_cache.peek(path);
    }

    bool Manager::removeSample(int id)
    {
        std::unique_lock lock(m_registryMutex);

        auto it = m_sampleRegistry.find(id);

        if (it != m_sampleRegistry.end())
        {
            m_cache.erase(it->second.registryKey);
            m_sampleRegistry.erase(id);
            return true;
        }
        return false;
    }

    std::vector<int> Manager::getAllSampleIds() const
    {
        std::shared_lock lock(m_registryMutex);

        std::vector<int> ids;
        ids.reserve(m_sampleRegistry.size()); // Optional optimization

        for (const auto &[id, entry] : m_sampleRegistry)
        {
            ids.push_back(id);
        }

        return ids;
    }

    bool dtracker::sample::Manager::contains(const std::string &path) const
    {
        std::shared_lock lock(m_registryMutex);

        return m_cache.contains(path);
    }
} // namespace dtracker::sample