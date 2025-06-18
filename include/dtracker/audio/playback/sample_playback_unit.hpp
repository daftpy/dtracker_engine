#pragma once

#include <dtracker/audio/playback/playback_unit.hpp>
#include <dtracker/sample/types.hpp>
#include <memory>
#include <vector>

namespace dtracker::audio::playback
{
    /// A playback unit that plays a single, non-looping audio sample from a
    /// buffer. It keeps track of its own playback position and reports when
    /// it's finished.
    class SamplePlaybackUnit : public PlaybackUnit
    {
      public:
        /// Default constructor for pre-allocation in an object pool.
        SamplePlaybackUnit();

        /// Constructs and initializes the unit with a specific sample
        /// descriptor.
        explicit SamplePlaybackUnit(sample::types::SampleDescriptor descriptor);

        /// Renders the next chunk of audio from the sample buffer.
        void render(float *buffer, unsigned int frames,
                    unsigned int channels) override;

        /// Returns true if the playback position has reached the end of the
        /// sample.
        bool isFinished() const override;

        /// Resets the playback position to the beginning of the sample.
        void reset() override;

        /// Re-initializes a recycled unit with a new sample for playback.
        void reinitialize(
            const dtracker::sample::types::SampleDescriptor &descriptor);

        /// Gets a const reference to the underlying PCM audio data.
        const std::vector<float> &data() const;

        /// Gets the sample rate of the audio data.
        unsigned int sampleRate() const;

        /// A public flag for debugging to track pool checkout status.
        bool isCheckedOut{false};

      private:
        /// Holds the shared pointer to the PCM data and its metadata.
        sample::types::SampleDescriptor m_descriptor;
        /// The current read position in the sample, measured in total samples
        /// (not frames).
        size_t m_position = 0;
    };

    /// A factory function for easily creating a unique_ptr to a
    /// SamplePlaybackUnit.
    std::unique_ptr<SamplePlaybackUnit>
    makePlaybackUnit(sample::types::SampleDescriptor descriptor);

} // namespace dtracker::audio::playback