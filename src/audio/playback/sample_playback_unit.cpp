#include <algorithm>
#include <dtracker/audio/playback/sample_playback_unit.hpp>
#include <iostream>

namespace dtracker::audio::playback
{

    SamplePlaybackUnit::SamplePlaybackUnit(
        std::shared_ptr<const SampleData> data)
        : m_data(std::move(data))
    {
        // m_data is expected to be even-length and stereo-safe,
        // handled during creation in SampleData constructor.
    }

    void SamplePlaybackUnit::render(float *buffer, unsigned int frames,
                                    unsigned int channels)
    {
        if (!m_data || channels != 2)
        {
            // Fill with silence if no data or unsupported channel config
            std::fill(buffer, buffer + (frames * channels), 0.0f);
            return;
        }

        const auto &samples = m_data->data();
        const size_t samplesToWrite = frames * channels;
        const size_t samplesRemaining =
            samples.size() > m_position ? samples.size() - m_position : 0;
        const size_t actualSamples = std::min(samplesToWrite, samplesRemaining);

        // Copy available portion into output buffer
        std::copy_n(samples.begin() + m_position, actualSamples, buffer);
        m_position += actualSamples;

        // Fill remaining with silence if we ran out early
        if (actualSamples < samplesToWrite)
        {
            std::fill(buffer + actualSamples, buffer + samplesToWrite, 0.0f);
        }
    }

    bool SamplePlaybackUnit::isFinished() const
    {
        return !m_data || m_position >= m_data->data().size();
    }

    void SamplePlaybackUnit::reset()
    {
        m_position = 0;
    }

    const std::vector<float> &SamplePlaybackUnit::data() const
    {
        return m_data->data();
    }

    unsigned int SamplePlaybackUnit::sampleRate() const
    {
        return m_data ? m_data->sampleRate() : 44100; // fallback if null
    }

    std::unique_ptr<SamplePlaybackUnit>
    makePlaybackUnit(std::shared_ptr<const dtracker::audio::SampleData> data)
    {
        return std::make_unique<SamplePlaybackUnit>(std::move(data));
    }

} // namespace dtracker::audio::playback
