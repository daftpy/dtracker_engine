#pragma once

#include <dtracker/audio/i_engine.hpp>
#include <dtracker/audio/i_playback_manager.hpp>
#include <dtracker/audio/playback/unit_pool.hpp>
#include <dtracker/sample/i_manager.hpp>

namespace dtracker::audio
{
    /// An implementation of the IEngine interface using the RtAudio
    /// library.
    class PlaybackManager : public IPlaybackManager
    {
      public:
        explicit PlaybackManager(IEngine *engine,
                                 sample::IManager *sampleManager);

        void
        playSample(std::unique_ptr<playback::SamplePlaybackUnit> unit) override;

        // Stops current playback if active
        void stopPlayback() override;

        // Returns whether the audio stream is currently running
        bool isPlaying() const override;

        /// Acquires a unit from the pool, reinitializes it, and plays it.
        void playSample(const sample::types::SampleDescriptor &descriptor);

      private:
        IEngine *m_engine{nullptr};
        sample::IManager *m_sampleManager{nullptr};

        playback::UnitPool m_unitPool;
    };
} // namespace dtracker::audio
