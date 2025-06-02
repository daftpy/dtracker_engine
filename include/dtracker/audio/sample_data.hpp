#pragma once
#include <vector>

namespace dtracker::audio
{
    class SampleData
    {
      public:
        SampleData(std::vector<float> pcm, unsigned int rate)
            : m_pcm(std::move(pcm)), m_sampleRate(rate)
        {
            if (m_pcm.size() % 2 != 0)
                m_pcm.push_back(
                    0.0f); // Ensure even-length to guarantee stereo-safety
        }

        const std::vector<float> &data() const
        {
            return m_pcm;
        }
        unsigned int sampleRate() const
        {
            return m_sampleRate;
        }

      private:
        std::vector<float> m_pcm;
        unsigned int m_sampleRate;
    };
} // namespace dtracker::audio
