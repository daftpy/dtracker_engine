#include "audio/playback_manager.hpp"

#include "audio/playback/tone_playback.hpp"

namespace dtracker::audio
{
    PlaybackManager::PlaybackManager(Engine *engine) : m_engine(engine) {}

    void PlaybackManager::playTestTone(float freq)
    {
        if (!m_engine)
            return;

        auto tone = std::make_unique<playback::TonePlayback>(freq, 44100.0f);
        m_engine->setPlaybackUnit(std::move(tone));
        m_engine->start();
    }

    void PlaybackManager::stopPlayback()
    {
        if (m_engine)
            m_engine->stop();
    }

    bool PlaybackManager::isPlaying() const
    {
        return m_engine && m_engine->isStreamRunning();
    }
} // namespace dtracker::audio
