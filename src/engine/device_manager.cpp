#include "engine/device_manager.hpp"

#include <RtAudio.h>

#include <iostream>

namespace dtracker::engine
{

    // Initializes DeviceManager with a pointer to an
    // existing RtAudio instance
    DeviceManager::DeviceManager(RtAudio *audio) : m_audio(audio)
    {
        std::cout << "DeviceManager: Initialized\n";

        // Defensive check to ensure the RtAudio pointer is valid
        if (!m_audio)
        {
            std::cerr << "DeviceManager: RtAudio pointer is null!\n";
            m_currentDeviceInfo = std::nullopt;
            return;
        }

        // Get the default output device ID from RtAudio
        unsigned int deviceId = m_audio->getDefaultOutputDevice();
        RtAudio::DeviceInfo info = m_audio->getDeviceInfo(deviceId);

        // Check if the default device has output channels; if not, search for a
        // fallback
        if (info.outputChannels == 0)
        {
            std::cout << "Default device has no output channels. Searching for "
                         "fallback...\n";
            auto fallbackId = findUsableOutputDevice();

            if (!fallbackId)
            {
                std::cout << "No usable output devices found.\n";
                m_currentDeviceInfo = std::nullopt;
                return;
            }

            // Use the fallback device instead
            deviceId = *fallbackId;
            info = m_audio->getDeviceInfo(deviceId);
        }

        // Store the selected device info
        m_currentDeviceInfo = info;

        // Log details about the selected device
        std::cout << "Selected output device: " << info.name << "\n"
                  << "Output Channels: " << info.outputChannels << "\n"
                  << "Sample Rates: ";
        for (const auto &rate : info.sampleRates)
            std::cout << rate << " ";
        std::cout << "\n";
    }

    // Attempts to find the first output-capable device
    std::optional<unsigned int> DeviceManager::findUsableOutputDevice()
    {
        // Get all available device IDs
        std::vector<unsigned int> ids = m_audio->getDeviceIds();

        // Iterate through them to find a device with at least one output
        // channel
        for (unsigned int id : ids)
        {
            RtAudio::DeviceInfo info = m_audio->getDeviceInfo(id);

            if (info.outputChannels > 0)
            {
                return id;
            }
        }

        // No usable output device found
        return std::nullopt;
    }

    // Returns device information for a specific device ID
    RtAudio::DeviceInfo DeviceManager::getDeviceInfo(unsigned int id) const
    {
        return m_audio->getDeviceInfo(id);
    }

    // Returns the currently selected device info, if one was successfully
    // initialized
    std::optional<RtAudio::DeviceInfo> DeviceManager::currentDeviceInfo() const
    {
        return m_currentDeviceInfo;
    }

} // namespace dtracker::engine
