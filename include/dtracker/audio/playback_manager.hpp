#pragma once

#include <dtracker/audio/engine.hpp>
#include <dtracker/audio/playback/playback_unit.hpp>
#include <dtracker/audio/sample_manager.hpp>
#include <memory>

namespace dtracker::audio
{
    class PlaybackManager
    {
      public:
        // Constructs the manager with a reference to an existing Engine (not
        // owned)
        explicit PlaybackManager(Engine *engine, SampleManager *sampleManager);

        // Plays a test sine tone with the given frequency (default is 440Hz)
        void playTestTone(float freq = 440.0f);

        // Plays a sample with a given sample rate
        void playSample(std::vector<float> pcmData, unsigned int sampleRate);

        // Stops current playback if active
        void stopPlayback();

        // Returns whether the audio stream is currently running
        bool isPlaying() const;

      private:
        Engine *m_engine{nullptr}; // Not owned
        SampleManager *m_sampleManager;
        std::vector<std::unique_ptr<playback::PlaybackUnit>> m_activeUnits;
    };
} // namespace dtracker::audio
