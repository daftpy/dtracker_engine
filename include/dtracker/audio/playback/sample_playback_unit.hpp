#pragma once

#include <dtracker/audio/playback/playback_unit.hpp>
#include <dtracker/audio/sample_data.hpp>
#include <dtracker/sample/types.hpp>
#include <memory>
#include <vector>

namespace dtracker::audio::playback
{
    /**
     * SamplePlaybackUnit plays back audio from a shared SampleData buffer.
     * It is safe to have multiple units reading the same SampleData in
     * parallel.
     */
    class SamplePlaybackUnit : public PlaybackUnit
    {
      public:
        /**
         * Constructs a SamplePlaybackUnit using a shared SampleData buffer.
         * The data is assumed to be stereo interleaved and even-length.
         */
        // explicit SamplePlaybackUnit(std::shared_ptr<const SampleData> data);
        explicit SamplePlaybackUnit(sample::types::SampleDescriptor descriptor);

        /**
         * Renders audio from the sample buffer into the output buffer.
         * Will mix or write into the output depending on remaining data.
         */
        void render(float *buffer, unsigned int frames,
                    unsigned int channels) override;

        /**
         * Returns true if all samples have been played back.
         */
        bool isFinished() const override;

        /**
         * Resets playback to the beginning of the sample.
         */
        void reset() override;

        /**
         * Returns a read-only reference to the sample data buffer.
         */
        const std::vector<float> &data() const;

        /**
         * Returns the sample rate associated with this sample.
         */
        unsigned int sampleRate() const;

      private:
        sample::types::SampleDescriptor m_descriptor;
        size_t m_position = 0; // Current playback position (sample index)
    };

    std::unique_ptr<SamplePlaybackUnit>
    makePlaybackUnit(sample::types::SampleDescriptor descriptor);

} // namespace dtracker::audio::playback
