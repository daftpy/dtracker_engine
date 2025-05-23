#include "audio/playback_manager.hpp"

#include "audio/playback/tone_playback.hpp"


namespace dtracker::audio
{
    // Initializes PlaybackManager with an Engine pointer
    PlaybackManager::PlaybackManager(Engine *engine) : m_engine(engine) {}

    // Creates a sine tone playback unit and starts the engine
    void PlaybackManager::playTestTone(float freq)
    {
        if (!m_engine)
            return;

        // Create a new tone generator with specified frequency and sample rate
        auto tone = std::make_unique<playback::TonePlayback>(freq, 44100.0f);

        // Provide the unit to the engine and start playback
        m_engine->setPlaybackUnit(std::move(tone));
        m_engine->start();
    }

    // Stops playback by calling Engine's stop
    void PlaybackManager::stopPlayback()
    {
        if (m_engine)
            m_engine->stop();
    }

    // Returns true if Engine is currently streaming audio
    bool PlaybackManager::isPlaying() const
    {
        return m_engine && m_engine->isStreamRunning();
    }
} // namespace dtracker::audio
