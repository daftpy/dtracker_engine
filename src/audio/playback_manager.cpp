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
            std::cout << "engine, trackmanager, or samplemanager is not "
                         "initialized\n";
            return;
        }

        // --- SETUP PHASE (Done safely on a non-real-time thread) ---

        // 1. Get the track's data from the TrackManager.
        auto trackDataPtr = m_trackManager->getTrack(trackId);
        if (!trackDataPtr)
        {
            std::cout << "Track " << trackId << " does not exist\n";
            // Track doesn't exist.
            return;
        }

        std::cout
            << "engine, managers, and track exist, creating playback unit\n";
        // 2. Create the top-level player for this track.
        auto trackPlaybackUnit =
            std::make_unique<playback::TrackPlaybackUnit>();
        trackPlaybackUnit->setVolume(trackDataPtr->volume);
        trackPlaybackUnit->setPan(trackDataPtr->pan);
        trackPlaybackUnit->setLooping(true);

        // 3. Build a single "blueprint" containing all unique samples needed
        // for the entire track.
        playback::SampleBlueprint blueprint;
        for (const auto &pattern : trackDataPtr->patterns)
        {
            for (int sampleId : pattern.steps)
            {
                std::cout << "SAMPLES FOUND IN STEPS\n";
                // If the blueprint doesn't already have this sample...
                if (sampleId >= 0 &&
                    blueprint.find(sampleId) == blueprint.end())
                {
                    // ...fetch it from the sample manager and add it.
                    if (auto descOpt = m_sampleManager->getSample(sampleId))
                    {
                        std::cout << "BLUE PRINT UPDATED\n";
                        blueprint[sampleId] = std::move(*descOpt);
                    }
                }
            }
        }

        // 4. Create a playable unit for each pattern in the track's sequence.
        for (const auto &pattern : trackDataPtr->patterns)
        {
            auto patternUnit = std::make_unique<playback::PatternPlaybackUnit>(
                pattern,
                blueprint,   // Give it the blueprint of all needed samples.
                &m_unitPool, // Give it access to the pool to create players.
                m_engine->getSettings()
                    .sampleRate // Give it the sample rate for timing.
            );
            trackPlaybackUnit->addUnit(std::move(patternUnit));
        }

        // --- PLAYBACK ---

        // 5. The fully constructed track is ready. Add it to the main engine
        // mixer.
        m_engine->mixerUnit()->clear(); // Clear any previous playback.
        m_engine->mixerUnit()->addUnit(std::move(trackPlaybackUnit));
    }

    void dtracker::audio::PlaybackManager::setLooping(bool shouldLoop)
    {
        m_isLooping = shouldLoop;
    }

} // namespace dtracker::audio
