#include <dtracker/audio/playback/sample_playback_unit.hpp>
#include <dtracker/audio/sample_manager.hpp>
#include <iostream>

namespace dtracker::audio
{

    int SampleManager::addSample(std::vector<float> pcm, unsigned int rate)
    {
        auto sample = std::make_shared<SampleData>(std::move(pcm),
                                                   rate); // Wrap in shared_ptr
        int id = m_nextId++;                              // Assign unique ID
        m_samples[id] = std::move(sample);                // Store in map
        return id;
    }

    std::shared_ptr<const SampleData>
    SampleManager::getSampleData(int sampleId) const
    {
        auto it = m_samples.find(sampleId); // Lookup sample by ID
        return (it != m_samples.end()) ? it->second
                                       : nullptr; // Return shared_ptr or null
    }

    bool SampleManager::removeSample(int sampleId)
    {
        bool erased = m_samples.erase(sampleId) > 0; // Attempt erase by ID
        std::cout << "SampleManager: removeSample(" << sampleId << ") -> "
                  << (erased ? "success" : "not found") << "\n";
        return erased;
    }

    std::vector<int> SampleManager::allSampleIds()
    {
        std::vector<int> ids;
        ids.reserve(m_samples.size());
        for (const auto &[id, _] : m_samples)
            ids.push_back(id); // Collect all keys
        return ids;
    }

} // namespace dtracker::audio
