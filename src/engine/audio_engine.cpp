#include "engine/audio_engine.hpp"

#include <iostream>

namespace dtracker::engine
{

    AudioEngine::AudioEngine() : m_audio(std::make_unique<RtAudio>())
    {
        std::cout << "AudioEngine: Initialized\n";
    }

    bool AudioEngine::start()
    {
        std::cout << "AudioEngine: Engine started\n";
        return true;
    }

    void AudioEngine::stop()
    {
        std::cout << "AudioEngine: Engine stopped\n";
    }

    std::optional<unsigned int> AudioEngine::getAvailableOutputDeviceId()
    {
        return std::optional<unsigned int>(10);
    }

    void AudioEngine::loadSample()
    {
        // TODO: Load sample
    }

} // namespace dtracker::engine