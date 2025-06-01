#pragma once

namespace dtracker::audio::playback
{
    /**
     * Abstract interface for audio-producing units that can render sound
     * into a shared output buffer.
     *
     * This interface allows the audio engine to treat all sound generators
     * (tones, samples, instruments, etc.) the same. Each derived class must
     * define how it fills the output buffer and when playback has completed.
     */
    class PlaybackUnit
    {
      public:
        virtual ~PlaybackUnit() = default;

        /**
         * Renders audio into the output buffer.
         *
         * output -    Pointer to the start of the (interleaved) output
         * buffer.
         *
         * nFrames -   Number of audio frames to generate.
         * channels -  Number of audio channels (e.g., 2 for stereo).
         *
         * The buffer is expected to already be zeroed out with memset, and
         * renderers should do their part by writing or mixing into it.
         */
        virtual void render(float *output, unsigned int nFrames,
                            unsigned int channels) = 0;

        virtual void reset() = 0;

        /**
         * Indicates whether playback has finished.
         *
         * Returns true if the unit has no more audio to produce or false
         * otherwise. Can be used to remove or recycle playback units.
         */
        virtual bool isFinished() const = 0;
    };
} // namespace dtracker::audio::playback
