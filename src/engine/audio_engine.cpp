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
        std::cout << "AudioEngine: Starting...\n";

        // Get the default system audio device
        unsigned int deviceId = m_audio->getDefaultOutputDevice();
        RtAudio::DeviceInfo info = m_audio->getDeviceInfo(deviceId);

        // Search for a fallback audio device if lacking channels
        if (info.outputChannels == 0)
        {
            std::cout << "Default device has no output channels. Searching for "
                         "fallback...\n";
            auto fallbackId = findUsableOutputDevice();

            // Return false if no suitable callback is found
            if (!fallbackId)
            {
                std::cout << "No usable output devices found.\n";
                return false;
            }
            deviceId = *fallbackId;
            info = m_audio->getDeviceInfo(deviceId);
        }

        std::cout << "Using device: " << info.name << " (" << deviceId << ")\n";
        std::cout << "Output channels: " << info.outputChannels << "\n";

        // TODO: open stream
        return true;
    }

    void AudioEngine::stop()
    {
        std::cout << "AudioEngine: Engine stopped\n";
    }

    std::optional<unsigned int> AudioEngine::findUsableOutputDevice()
    {
        // Iterate through all available devices
        std::vector<unsigned int> ids = m_audio->getDeviceIds();
        for (unsigned int id : ids)
        {
            RtAudio::DeviceInfo info = m_audio->getDeviceInfo(id);

            // Check if the device has output channels
            if (info.outputChannels > 0)
            {
                return id;
            }
        }

        // No usable output device found
        return std::nullopt;
    }

    RtAudio::DeviceInfo AudioEngine::getDeviceInfo(unsigned int id) const
    {
        // Return the device info for a given device ID
        return m_audio->getDeviceInfo(id);
    }

} // namespace dtracker::engine