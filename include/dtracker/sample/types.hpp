#pragma once
#include <cstdint>
#include <dtracker/audio/types.hpp>
#include <memory>
#include <string>
#include <vector>

namespace dtracker::sample::types
{

    struct SampleMetadata
    {
        // The original sample rate of the audio file (e.g., 44100, 48000).
        unsigned int sourceSampleRate;

        // The original bit depth (e.g., 16, 24).
        unsigned int bitDepth;
    };

    class SampleDescriptor
    {
      public:
        SampleDescriptor(
            int id, const std::shared_ptr<const audio::types::PCMData> pcmData,
            const SampleMetadata &metadata)
            : m_registryId(id), m_pcmData(std::move(pcmData)),
              m_metadata(metadata)
        {
        }

        // Gets a shared pointer to the raw audio data.
        const std::shared_ptr<const audio::types::PCMData> &pcmData() const
        {
            return m_pcmData;
        }

        // Gets the metadata (sample rate, loop points, etc.).
        const SampleMetadata &metadata() const
        {
            return m_metadata;
        }

      private:
        int m_registryId{-1};
        std::shared_ptr<const audio::types::PCMData>
            m_pcmData;             // Shared ownership of the large audio buffer
        SampleMetadata m_metadata; // Owns a copy of the lightweight metadata
    };

    struct SampleEntry
    {
        int id;
        std::string registryKey;
        SampleMetadata metaData;
    };
} // namespace dtracker::sample::types