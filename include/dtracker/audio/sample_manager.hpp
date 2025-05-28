#pragma once

#include <dtracker/audio/playback/playback_unit.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

namespace dtracker::audio
{
    class SampleManager
    {
      public:
        SampleManager() = default;

        int addSample(std::vector<float> pcm, unsigned int rate);
        std::unique_ptr<playback::PlaybackUnit> getSample(int sampleId);
        bool removeSample(int sampleId);
        std::vector<int> allSampleIds();

      private:
        int m_nextId{0};
        std::unordered_map<int, std::unique_ptr<playback::PlaybackUnit>>
            m_samples;
    };
} // namespace dtracker::audio