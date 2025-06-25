#include <dtracker/audio/playback/pattern_playback_unit.hpp>
#include <dtracker/audio/playback/sample_playback_unit.hpp>
#include <dtracker/audio/playback/track_playback_unit.hpp>
#include <dtracker/audio/playback_manager.hpp>

namespace dtracker::audio
{
    // Initializes PlaybackManager with an Engine pointer
    PlaybackManager::PlaybackManager(IEngine *engine,
                                     sample::IManager *sampleManager,
                                     tracker::ITrackManager *trackManager)
        : m_engine(engine), m_sampleManager(sampleManager),
          m_trackManager(trackManager), m_unitPool(128)
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
        // Ensure the engine dependency is valid.
        if (!m_engine)
            return;

        // Acquire a recycled player from the pool. This is fast and
        // allocation-free.
        auto unitPtr = m_unitPool.acquire();

        // Check if the pool had an available object.
        if (unitPtr)
        {
            // Re-initialize the recycled player with the new sample's data.
            unitPtr->reinitialize(descriptor);

            // Add the ready-to-go unit to the mixer for playback.
            m_engine->mixerUnit()->addUnit(std::move(unitPtr));
        }
    }

    void dtracker::audio::PlaybackManager::playTrack(int trackId)
    {
        if (!m_engine || !m_trackManager || !m_sampleManager)
        {
            return;
        }

        auto trackPlayer = buildTrackPlayer(trackId);
        if (!trackPlayer)
            return;

        m_engine->mixerUnit()->clear();
        m_engine->mixerUnit()->addUnit(std::move(trackPlayer));
    }

    void dtracker::audio::PlaybackManager::playAllTracks()
    {
        if (!m_engine || !m_trackManager || !m_sampleManager)
        {
            return;
        }

        m_engine->mixerUnit()->clear();

        std::vector<int> allTrackIds = m_trackManager->getAllTrackIds();

        for (int trackId : allTrackIds)
        {
            if (auto trackPlayer = buildTrackPlayer(trackId))
            {
                m_engine->mixerUnit()->addUnit(std::move(trackPlayer));
            }
        }
    }

    void dtracker::audio::PlaybackManager::setLoopPlayback(bool shouldLoop)
    {
        if (m_engine != nullptr)
        {
            m_engine->proxyUnit()->setIsLooping(shouldLoop);
        }
    }

    bool dtracker::audio::PlaybackManager::loopPlayback() const
    {
        if (m_engine != nullptr)
        {
            return m_engine->proxyUnit()->isLooping();
        }
        return false;
    }

    std::unique_ptr<playback::TrackPlaybackUnit>
    dtracker::audio::PlaybackManager::buildTrackPlayer(int trackId)
    {
        // 1. Get the track's data from the TrackManager.
        auto trackDataPtr = m_trackManager->getTrack(trackId);
        if (!trackDataPtr)
        {
            return nullptr; // Return null if track doesn't exist
        }

        // 2. Create the top-level player for this track.
        auto trackPlaybackUnit =
            std::make_unique<playback::TrackPlaybackUnit>();
        trackPlaybackUnit->setVolume(trackDataPtr->volume);
        trackPlaybackUnit->setPan(trackDataPtr->pan);
        // trackPlaybackUnit->setLooping(m_isLooping);

        // 3. Build the blueprint for all patterns in this track.
        playback::SampleBlueprint blueprint;
        for (const auto &pattern : trackDataPtr->patterns)
        {
            for (int sampleId : pattern.steps)
            {
                if (sampleId >= 0 &&
                    blueprint.find(sampleId) == blueprint.end())
                {
                    if (auto descOpt = m_sampleManager->getSample(sampleId))
                    {
                        blueprint[sampleId] = std::move(*descOpt);
                    }
                }
            }
        }

        // 4. Create a playable unit for each pattern in the track's sequence.
        for (const auto &pattern : trackDataPtr->patterns)
        {
            auto patternUnit = std::make_unique<playback::PatternPlaybackUnit>(
                pattern, blueprint, &m_unitPool,
                m_engine->getSettings().sampleRate);
            trackPlaybackUnit->addUnit(std::move(patternUnit));
        }

        return trackPlaybackUnit;
    }

} // namespace dtracker::audio
