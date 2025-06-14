#pragma once

#include <dtracker/audio/playback/sample_playback_unit.hpp>
#include <memory>

namespace dtracker::audio
{
    /// Defines the high-level API for controlling audio playback.
    /// This acts as a facade, simplifying complex playback logic for clients.
    class IPlaybackManager
    {
      public:
        virtual ~IPlaybackManager() = default;

        /// Takes ownership of and begins playback of a single sample unit.
        virtual void
        playSample(std::unique_ptr<playback::SamplePlaybackUnit> unit) = 0;

        /// Immediately stops all currently playing audio.
        virtual void stopPlayback() = 0;

        /// Returns true if any audio is currently playing through the engine.
        virtual bool isPlaying() const = 0;
    };

} // namespace dtracker::audio