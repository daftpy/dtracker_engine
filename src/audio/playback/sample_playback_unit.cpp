#include <algorithm>
#include <dtracker/audio/playback/sample_playback_unit.hpp>
#include <iostream>

namespace dtracker::audio::playback
{
    dtracker::audio::playback::SamplePlaybackUnit::SamplePlaybackUnit(
        sample::types::SampleDescriptor descriptor)
        : m_descriptor(std::move(descriptor))
    {
    }

    void SamplePlaybackUnit::render(float *buffer, unsigned int frames,
                                    unsigned int channels)
    {
        const auto pcmPtr = m_descriptor.pcmData();
        if (!pcmPtr || channels != 2)
        {
            // Fill with silence if no data or unsupported channel config
            std::fill(buffer, buffer + (frames * channels), 0.0f);
            return;
        }

        const auto &samples = *pcmPtr;
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
        const auto pcmPtr = m_descriptor.pcmData();
        return !pcmPtr || m_position >= pcmPtr->size();
    }

    void SamplePlaybackUnit::reset()
    {
        m_position = 0;
    }

    const std::vector<float> &SamplePlaybackUnit::data() const
    {
        return *m_descriptor.pcmData();
    }

    unsigned int SamplePlaybackUnit::sampleRate() const
    {
        return m_descriptor.metadata().sourceSampleRate;
    }

    void dtracker::audio::playback::SamplePlaybackUnit::reinitialize(
        const dtracker::sample::types::SampleDescriptor &descriptor)
    {
        m_descriptor = descriptor;
        reset();
    }

    std::unique_ptr<SamplePlaybackUnit>
    makePlaybackUnit(sample::types::SampleDescriptor descriptor)
    {
        return std::make_unique<SamplePlaybackUnit>(std::move(descriptor));
    }

} // namespace dtracker::audio::playback
