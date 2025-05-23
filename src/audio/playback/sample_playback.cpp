#include <algorithm>
#include <dtracker/audio/playback/sample_playback.hpp>


namespace dtracker::audio::playback
{

    SamplePlayback::SamplePlayback(std::vector<float> samples,
                                   unsigned int sampleRate)
        : m_samples(std::move(samples)), m_sampleRate(sampleRate)
    {
        // Ensure the data is interleaved stereo (even number of floats)
        if (m_samples.size() % 2 != 0)
            m_samples.push_back(0.0f); // Pad with silence if needed
    }

    void SamplePlayback::render(float *buffer, unsigned int frames,
                                unsigned int channels)
    {
        if (channels != 2)
        {
            // Only stereo is supported for now
            std::fill(buffer, buffer + (frames * channels), 0.0f);
            return;
        }

        size_t samplesToWrite = frames * channels;
        size_t samplesRemaining = m_samples.size() - m_position;
        size_t actualSamples = std::min(samplesToWrite, samplesRemaining);

        // Copy available samples
        std::copy_n(m_samples.begin() + m_position, actualSamples, buffer);
        m_position += actualSamples;

        // Fill the raminder with zeroes if finished early
        if (actualSamples < samplesToWrite)
        {
            std::fill(buffer + actualSamples, buffer + samplesToWrite, 0.0f);
        }
    }

    bool SamplePlayback::isFinished() const
    {
        return m_position >= m_samples.size();
    }

} // namespace dtracker::audio::playback
