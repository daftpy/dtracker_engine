#include <dtracker/audio/playback/sample_playback.hpp> // Needed to construct sample units
#include <dtracker/audio/sample_manager.hpp>
#include <iostream>

namespace dtracker::audio
{

    int SampleManager::addSample(std::vector<float> pcm, unsigned int rate)
    {
        // Create a new SamplePlayback unit with the provided PCM and sample
        // rate
        auto unit = std::make_unique<playback::SamplePlaybackUnit>(
            std::move(pcm), rate);

        // Store it in the map with a unique ID
        int id = m_nextId++;
        m_samples[id] = std::move(unit);

        std::cout << "Number of samples: " << m_samples.size() << "\n";

        // Return the ID for later reference
        return id;
    }

    // Returns a raw pointer to the unique pointer in the manager
    playback::PlaybackUnit *SampleManager::getSample(int sampleId)
    {
        auto it = m_samples.find(sampleId);
        if (it != m_samples.end())
            return it->second.get();
        return nullptr;
    }

    bool SampleManager::removeSample(int sampleId)
    {
        bool erased = m_samples.erase(sampleId) > 0;
        std::cout << "SampleManager: removeSample(" << sampleId << ") -> "
                  << (erased ? "success" : "not found") << "\n";
        return erased;
    }

    std::vector<int> SampleManager::allSampleIds()
    {
        std::vector<int> ids;
        ids.reserve(m_samples.size());

        // Collect all keys from the map
        for (const auto &[id, _] : m_samples)
            ids.push_back(id);

        return ids;
    }

} // namespace dtracker::audio
