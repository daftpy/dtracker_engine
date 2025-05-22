#include "engine/audio_engine.hpp"

#include <RtAudio.h>

#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace dtracker::engine
{
    int audioCallback(void *outputBuffer, void * /*inputBuffer*/,
                      unsigned int nFrames, double /*streamTime*/,
                      RtAudioStreamStatus status, void *userData)
    {
        if (status)
            std::cerr << "Stream underflow/overflow detected!\n";

        auto *sine = static_cast<audio::waves::SineWave *>(userData);
        float *buffer = static_cast<float *>(outputBuffer);

        const float twoPi = 2.0f * static_cast<float>(M_PI);
        const float phaseIncrement = twoPi * sine->frequency / sine->sampleRate;

        std::cerr << "Callback running: nFrames=" << nFrames << "\n";
        for (unsigned int i = 0; i < nFrames; ++i)
        {
            float sample = std::sin(sine->phase);
            sine->phase += phaseIncrement;
            if (sine->phase >= twoPi)
                sine->phase -= twoPi;

            // Write to both stereo channels
            buffer[2 * i] = sample;     // Left
            buffer[2 * i + 1] = sample; // Right
        }

        return 0;
    }

    AudioEngine::AudioEngine() : m_audio(std::make_unique<RtAudio>())
    {
        std::cout << "AudioEngine: Initialized\n";

        // Setup error callback
        m_audio->setErrorCallback(
            [](RtAudioErrorType type, const std::string &errorText)
            {
                std::cerr << "[RtAudio Error] (" << static_cast<int>(type)
                          << "): " << errorText << "\n";
            });
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
        m_currentDeviceInfo = info;
        m_started = true;

        std::cout << "Using device: " << info.name << " (" << deviceId << ")\n";
        std::cout << "Output channels: " << info.outputChannels << "\n";

        m_sine = std::make_unique<audio::waves::SineWave>();
        m_sine->sampleRate = static_cast<float>(m_settings.sampleRate);

        // Open the stream
        return openAndStartStream(deviceId);
    }

    void AudioEngine::stop()
    {
        if (!m_started)
            return;

        if (m_audio->isStreamRunning())
        {
            auto err = m_audio->stopStream();
            if (err != RTAUDIO_NO_ERROR)
            {
                std::cerr << "AudioEngine: Failed to stop stream ("
                          << static_cast<int>(err)
                          << "): " << m_audio->getErrorText() << "\n";
            }
        }

        if (m_audio->isStreamOpen())
        {
            // No return value — errors will go through error callback
            m_audio->closeStream();
        }

        m_started = false;
        std::cout << "AudioEngine: Engine stopped\n";
    }

    bool AudioEngine::openAndStartStream(unsigned int deviceId)
    {
        RtAudio::StreamParameters outputParams;
        outputParams.deviceId = deviceId;
        outputParams.nChannels = m_settings.outputChannels;
        outputParams.firstChannel = 0;

        // Open stream
        auto err = m_audio->openStream(
            &outputParams, nullptr, RTAUDIO_FLOAT32, m_settings.sampleRate,
            &m_settings.bufferFrames, &audioCallback, m_sine.get());

        if (err != RTAUDIO_NO_ERROR)
        {
            std::cerr << "AudioEngine: Failed to open stream: "
                      << m_audio->getErrorText() << "\n";
            return false;
        }

        // Start stream
        err = m_audio->startStream();
        if (err != RTAUDIO_NO_ERROR)
        {
            std::cerr << "AudioEngine: Failed to start stream: "
                      << m_audio->getErrorText() << "\n";
            return false;
        }

        std::cout << "AudioEngine: Stream successfully started\n";
        return true;
    }

    bool AudioEngine::isStreamOpen() const
    {
        return m_audio->isStreamOpen();
    }

    bool AudioEngine::isStreamRunning() const
    {
        return m_audio->isStreamRunning();
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

    RtAudio::DeviceInfo dtracker::engine::AudioEngine::currentDeviceInfo() const
    {
        return m_currentDeviceInfo;
    }

} // namespace dtracker::engine