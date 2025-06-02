#pragma once
#include <vector>

namespace dtracker::audio
{
    class SampleData
    {
      public:
        SampleData(std::vector<float> data, unsigned int rate)
            : m_pcm(std::move(data)), m_rate(rate)
        {
        }

        const std::vector<float> &pcm() const
        {
            return m_pcm;
        }
        unsigned int rate() const
        {
            return m_rate;
        }

      private:
        std::vector<float> m_pcm;
        unsigned int m_rate;
    };
} // namespace dtracker::audio
