#pragma once

#include <dtracker/audio/i_engine.hpp>
#include <dtracker/audio/i_playback_manager.hpp>
#include <dtracker/audio/playback/track_playback_unit.hpp>
#include <dtracker/audio/playback/unit_pool.hpp>
#include <dtracker/sample/i_manager.hpp>
#include <dtracker/tracker/i_track_manager.hpp>

namespace dtracker::audio
{
    /// An implementation of the IEngine interface using the RtAudio
    /// library.
    class PlaybackManager : public IPlaybackManager
    {
      public:
        explicit PlaybackManager(IEngine *engine,
                                 sample::IManager *sampleManager,
                                 tracker::ITrackManager *trackManager);

        void
        playSample(std::unique_ptr<playback::SamplePlaybackUnit> unit) override;

        /// Immediately stops all audio and clears the main mixer.
        void stopPlayback() override;

        /// Checks if any audio is currently being produced by the engine.
        bool isPlaying() const override;

        /// Acquires a unit from the pool, reinitializes it, and plays it.
        void playSample(const sample::types::SampleDescriptor &descriptor);

        /// Builds and plays a single track by its ID, replacing any current
        /// playback.
        void playTrack(int trackId) override;

        /// Sets the master looping state for track playback.
        void setLoopPlayback(bool shouldLoop) override;

        /// Builds and plays all registered tracks simultaneously.
        void playAllTracks() override;

        /// Returns the master looping state.
        bool loopPlayback() const;

        /// Sets the master playback tempo in Beats Per Minute (BPM).
        void setBpm(float bpm) override;

        /// Gets the master playback tempo.
        float bpm() const;

      private:
        std::unique_ptr<playback::TrackPlaybackUnit>
        buildTrackPlayer(int trackId);
        std::atomic<float> m_bpm{120.0f}; // The master BPM value

        bool m_isLooping{true};

        IEngine *m_engine{nullptr};
        sample::IManager *m_sampleManager{nullptr};
        tracker::ITrackManager *m_trackManager{nullptr};

        playback::UnitPool m_unitPool;
    };
} // namespace dtracker::audio
