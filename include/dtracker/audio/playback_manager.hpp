#pragma once

#include <dtracker/audio/i_engine.hpp>
#include <dtracker/audio/i_playback_manager.hpp>

namespace dtracker::audio
{
    /// An implementation of the IEngine interface using the RtAudio
    /// library.
    class PlaybackManager : public IPlaybackManager
    {
      public:
        explicit PlaybackManager(IEngine *engine);

        void
        playSample(std::unique_ptr<playback::SamplePlaybackUnit> unit) override;

        // Stops current playback if active
        void stopPlayback() override;

        // Returns whether the audio stream is currently running
        bool isPlaying() const override;

      private:
        IEngine *m_engine{nullptr};
    };
} // namespace dtracker::audio
