#pragma once

#include <dtracker/audio/engine.hpp>
#include <dtracker/audio/playback/sample_playback_unit.hpp>
#include <dtracker/sample/manager.hpp>
#include <dtracker/tracker/types/active_pattern.hpp>
#include <memory>

namespace dtracker::audio
{
    class PlaybackManager
    {
      public:
        // Constructs the manager with a reference to an existing Engine (not
        // owned)
        explicit PlaybackManager(Engine *engine);

        void playSample(std::unique_ptr<playback::SamplePlaybackUnit> unit);

        // Stops current playback if active
        void stopPlayback();

        // Returns whether the audio stream is currently running
        bool isPlaying() const;

      private:
        Engine *m_engine{nullptr}; // Not owned
    };
} // namespace dtracker::audio
