#include <dtracker/audio/playback/sample_playback_unit.hpp>
#include <dtracker/audio/playback_manager.hpp>

namespace dtracker::audio
{
    // Initializes PlaybackManager with an Engine pointer
    PlaybackManager::PlaybackManager(Engine *engine) : m_engine(engine) {}

    void dtracker::audio::PlaybackManager::playSample(
        std::unique_ptr<playback::SamplePlaybackUnit> unit)
    {
        if (m_engine)
            m_engine->mixerUnit()->addUnit(std::move(unit));
    }

    // Stops playback by calling Engine's stop
    void PlaybackManager::stopPlayback()
    {
        if (m_engine)
            m_engine->mixerUnit()->clear(); // clears the delegate in the mixer
    }

    // Returns true if Engine is currently streaming audio
    bool PlaybackManager::isPlaying() const
    {
        return m_engine && m_engine->isStreamRunning() &&
               !m_engine->mixerUnit()->isFinished();
    }

} // namespace dtracker::audio
