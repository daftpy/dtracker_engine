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
          m_trackManager(trackManager), m_unitPool(128),
          // Create a pool of 128 buffers, each large enough for a common buffer
          // size.
          m_bufferPool(128, 1024)
    {

        m_masterWaveFormQueue = std::make_unique<
            rigtorp::SPSCQueue<playback::BufferPool::PooledBufferPtr>>(64);
        m_engine->mixerUnit()->setBufferPool(&m_bufferPool);
        m_engine->mixerUnit()->setWaveformQueue(m_masterWaveFormQueue.get());
    }

    void dtracker::audio::PlaybackManager::playSample(
        std::unique_ptr<playback::SamplePlaybackUnit> unit)
    {
        if (m_engine)
            m_engine->mixerUnit()->addUnit(std::move(unit));
    }

    // Update stopPlayback to clear the waveform queues.
    void PlaybackManager::stopPlayback()
    {
        if (m_engine)
        {
            m_engine->mixerUnit()->clear();
        }
        // Also clear out all the waveform resources.
        std::lock_guard<std::mutex> lock(m_waveformQueuesMutex);
        m_waveformQueues.clear();
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

    // Update playTrack to create a queue for the track being played.
    void PlaybackManager::playTrack(int trackId)
    {
        if (!m_engine || !m_trackManager || !m_sampleManager)
            return;

        // Before we start, clear everything.
        stopPlayback();

        // Create a new waveform queue for this track.
        {
            std::lock_guard<std::mutex> lock(m_waveformQueuesMutex);
            // The queue needs a capacity. 2 seconds of audio data is very safe.
            m_waveformQueues[trackId] = std::make_unique<
                rigtorp::SPSCQueue<playback::BufferPool::PooledBufferPtr>>(64);
        }

        // Pass the raw pointers to the new resources to the builder.
        auto trackPlayer = buildTrackPlayer(trackId, &m_bufferPool,
                                            m_waveformQueues[trackId].get());

        if (!trackPlayer)
            return;

        m_engine->mixerUnit()->addUnit(std::move(trackPlayer));
    }

    void dtracker::audio::PlaybackManager::playAllTracks()
    {
        if (!m_engine || !m_trackManager || !m_sampleManager)
        {
            return;
        }

        // Before we start, clear everything.
        stopPlayback();

        std::vector<int> allTrackIds = m_trackManager->getAllTrackIds();

        std::lock_guard<std::mutex> lock(m_waveformQueuesMutex);
        for (int trackId : allTrackIds)
        {
            m_waveformQueues[trackId] = std::make_unique<
                rigtorp::SPSCQueue<playback::BufferPool::PooledBufferPtr>>(64);
            if (auto trackPlayer = buildTrackPlayer(
                    trackId, &m_bufferPool, m_waveformQueues[trackId].get()))
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
    dtracker::audio::PlaybackManager::buildTrackPlayer(
        int trackId, playback::BufferPool *bufferPool,
        rigtorp::SPSCQueue<playback::BufferPool::PooledBufferPtr>
            *waveformQueue)
    {
        // 1. Get the track's data from the TrackManager.
        auto trackDataPtr = m_trackManager->getTrack(trackId);
        if (!trackDataPtr)
        {
            return nullptr; // Return null if track doesn't exist
        }

        // 2. Create the top-level player for this track.
        auto trackPlaybackUnit = std::make_unique<playback::TrackPlaybackUnit>(
            bufferPool, waveformQueue);

        trackPlaybackUnit->setVolume(trackDataPtr->volume);
        trackPlaybackUnit->setPan(trackDataPtr->pan);

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

    void PlaybackManager::setBpm(float bpm)
    {
        if (m_engine)
        {
            m_bpm.store(bpm, std::memory_order_relaxed);

            // Update the proxy so the audio thread sees the change.
            m_engine->proxyUnit()->setBpm(bpm);
        }
    }

    float PlaybackManager::bpm() const
    {
        return m_bpm;
    }

    rigtorp::SPSCQueue<playback::BufferPool::PooledBufferPtr> *
    PlaybackManager::getWaveformQueueForTrack(int trackId)
    {
        std::lock_guard<std::mutex> lock(m_waveformQueuesMutex);
        if (auto it = m_waveformQueues.find(trackId);
            it != m_waveformQueues.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    rigtorp::SPSCQueue<playback::BufferPool::PooledBufferPtr> *
    dtracker::audio::PlaybackManager::getMasterWaveformQueue()
    {
        std::lock_guard<std::mutex> lock(m_masterWaveFormQueueMutex);
        return m_masterWaveFormQueue.get();
    }

    playback::BufferPool *dtracker::audio::PlaybackManager::getBufferPool()
    {
        return &m_bufferPool;
    }

} // namespace dtracker::audio