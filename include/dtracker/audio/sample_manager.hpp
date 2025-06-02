#pragma once

#include <dtracker/audio/playback/playback_unit.hpp>
#include <dtracker/audio/sample_data.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

namespace dtracker::audio
{
    /**
     * Manages all loaded sample data in memory.
     * Each sample is stored once and can be referenced by ID.
     */
    class SampleManager
    {
      public:
        SampleManager() = default;

        /**
         * Adds a new sample to the manager and returns its assigned ID.
         *
         * - pcm: interleaved stereo float data
         * - rate: sample rate (e.g., 44100)
         */
        int addSample(std::vector<float> pcm, unsigned int rate);

        /**
         * Retrieves shared sample data by ID.
         * This allows callers to create new PlaybackUnits from the same PCM.
         *
         * Returns nullptr if not found.
         */
        std::shared_ptr<const SampleData> getSampleData(int sampleId) const;

        /**
         * Removes a sample from the manager by ID.
         * Returns true if the sample was found and removed.
         */
        bool removeSample(int sampleId);

        /**
         * Returns a list of all sample IDs currently stored.
         */
        std::vector<int> allSampleIds();

      private:
        int m_nextId{0}; ///< Auto-incrementing ID counter

        // Stores all sample data using shared_ptr for safe reuse across units
        std::unordered_map<int, std::shared_ptr<const SampleData>> m_samples;
    };
} // namespace dtracker::audio
