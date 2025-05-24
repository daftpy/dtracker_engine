#include <dtracker/audio/playback/sample_playback.hpp>
#include <dtracker/audio/playback/tone_playback.hpp>
#include <dtracker/audio/playback_manager.hpp>

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
    }

    void PlaybackManager::playSample(std::vector<float> pcmData,
                                     unsigned int sampleRate)
    {
        if (!m_engine)
            return;

        auto sampleUnit = std::make_unique<playback::SamplePlayback>(
            std::move(pcmData), sampleRate);
        m_engine->setPlaybackUnit(std::move(sampleUnit));
    }

    // Stops playback by calling Engine's stop
    void PlaybackManager::stopPlayback()
    {
        if (m_engine)
            m_engine->setPlaybackUnit(
                nullptr); // clears the delegate in the proxy
    }

    // Returns true if Engine is currently streaming audio
    bool PlaybackManager::isPlaying() const
    {
        return m_engine && m_engine->isStreamRunning() &&
               m_engine->proxyPlaybackUnit()->delegate() != nullptr;
    }

} // namespace dtracker::audio
