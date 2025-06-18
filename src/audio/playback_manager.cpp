#include <dtracker/audio/playback/sample_playback_unit.hpp>
#include <dtracker/audio/playback_manager.hpp>

namespace dtracker::audio
{
    // Initializes PlaybackManager with an Engine pointer
    PlaybackManager::PlaybackManager(IEngine *engine,
                                     sample::IManager *sampleManager)
        : m_engine(engine), m_sampleManager(sampleManager), m_unitPool(128)
    {
    }

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

    void dtracker::audio::PlaybackManager::playSample(
        const sample::types::SampleDescriptor &descriptor)
    {
        // // Ensure the engine dependency is valid.
        // if (!m_engine)
        //     return;

        // // 1. Acquire a recycled player from the pool. This is fast and
        // // allocation-free.
        // auto unitPtr = m_unitPool.acquire();

        // // 2. Check if the pool had an available object.
        // if (unitPtr)
        // {
        //     // 3. Re-initialize the recycled player with the new sample's
        //     data. unitPtr->reinitialize(descriptor);

        //     // 4. Add the ready-to-go unit to the mixer for playback.
        //     m_engine->mixerUnit()->addUnit(std::move(unitPtr));
        // }
    }

} // namespace dtracker::audio
