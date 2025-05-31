#include <dtracker/audio/playback/sample_playback.hpp>
#include <dtracker/audio/playback/tone_playback.hpp>
#include <dtracker/audio/playback_manager.hpp>

namespace dtracker::audio
{
    // Initializes PlaybackManager with an Engine pointer
    PlaybackManager::PlaybackManager(Engine *engine,
                                     SampleManager *sampleManager)
        : m_engine(engine), m_sampleManager(sampleManager)
    {
    }

    // Creates a sine tone playback unit and starts the engine
    void PlaybackManager::playTestTone(float freq)
    {
        if (!m_engine)
            return;

        auto tone = std::make_unique<playback::TonePlayback>(freq, 44100.0f);

        m_engine->proxyPlaybackUnit()->setDelegate(tone.get());
        m_activeUnits.push_back(std::move(tone));
    }

    void PlaybackManager::playSample(std::vector<float> pcmData,
                                     unsigned int sampleRate)
    {
        if (!m_engine)
            return;

        auto unit = std::make_unique<playback::SamplePlaybackUnit>(
            std::move(pcmData), sampleRate);
        m_engine->proxyPlaybackUnit()->setDelegate(unit.get());
        std::cout << "Playing sample\nVector Size: " << m_activeUnits.size()
                  << "\n";
        m_activeUnits.push_back(std::move(unit));
    }

    void PlaybackManager::playSampleById(int sampleId)
    {
        std::cout << "Playing sample by ID\n";
        if (!m_sampleManager || !m_engine)
            return;

        std::cout << "Retrieving sample by ID " << sampleId << "\n";
        auto unit = m_sampleManager->getSample(sampleId);
        if (!unit)
        {
            std::cout << "No sample found\n";
            return;
        }
        auto *mixer = m_engine->mixerUnit();
        if (!mixer)
        {
            std::cout << "No mixer unit found\n";
            return;
        }

        std::cout << "Adding sample to mixer " << sampleId << "\n";
        mixer->addUnit(unit);
    }

    // Stops playback by calling Engine's stop
    void PlaybackManager::stopPlayback()
    {
        if (m_engine)
            m_engine->proxyPlaybackUnit()->setDelegate(
                nullptr); // clears the delegate in the proxy
    }

    // Returns true if Engine is currently streaming audio
    bool PlaybackManager::isPlaying() const
    {
        return m_engine && m_engine->isStreamRunning() &&
               m_engine->proxyPlaybackUnit()->delegate() != nullptr;
    }

} // namespace dtracker::audio
