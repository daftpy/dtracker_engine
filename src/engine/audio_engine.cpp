#include "engine/audio_engine.hpp"

#include <RtAudio.h>

#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace dtracker::engine
{

    // Static callback used by RtAudio to fill the audio buffer
    int audioCallback(void *outputBuffer, void * /*inputBuffer*/,
                      unsigned int nFrames, double /*streamTime*/,
                      RtAudioStreamStatus status, void *userData)
    {
        // Notify of any stream underflow or overflow issues
        if (status)
            std::cerr << "Stream underflow/overflow detected!\n";

        // Cast the provided user data to a SineWave generator
        auto *sine = static_cast<audio::waves::SineWave *>(userData);
        float *buffer = static_cast<float *>(outputBuffer);

        const float twoPi = 2.0f * static_cast<float>(M_PI);
        const float phaseIncrement = twoPi * sine->frequency / sine->sampleRate;

        // Fill buffer with sine wave samples for both stereo channels
        std::cerr << "Callback running: nFrames=" << nFrames << "\n";
        for (unsigned int i = 0; i < nFrames; ++i)
        {
            float sample = std::sin(sine->phase);
            sine->phase += phaseIncrement;
            if (sine->phase >= twoPi)
                sine->phase -= twoPi;

            buffer[2 * i] = sample;     // Left channel
            buffer[2 * i + 1] = sample; // Right channel
        }

        return 0;
    }

    // Initializes audio system and device manager
    AudioEngine::AudioEngine()
        : m_audio(std::make_unique<RtAudio>()),
          m_deviceManager(m_audio.get()) // Pass raw pointer to DeviceManager
    {
        std::cout << "AudioEngine: Initialized\n";

        // Install error callback to handle RtAudio runtime issues
        m_audio->setErrorCallback(
            [](RtAudioErrorType type, const std::string &errorText)
            {
                std::cerr << "[RtAudio Error] (" << static_cast<int>(type)
                          << "): " << errorText << "\n";
            });
    }

    // Starts the audio engine, opening and running the stream if a device is
    // available
    bool AudioEngine::start()
    {
        std::cout << "AudioEngine: Starting...\n";

        auto currentDeviceInfo = m_deviceManager.currentDeviceInfo();

        // Only proceed if a valid device was found
        if (currentDeviceInfo.has_value())
        {
            auto &info = currentDeviceInfo.value();
            m_started = true;

            std::cout << "Using device: " << info.name << " (" << info.ID
                      << ")\n";
            std::cout << "Output channels: " << info.outputChannels << "\n";

            // Initialize sine wave generator for output
            m_sine = std::make_unique<audio::waves::SineWave>();
            m_sine->sampleRate = static_cast<float>(m_settings.sampleRate);

            // Open and start the audio stream
            return openAndStartStream(info.ID);
        }

        std::cerr << "AudioEngine: No valid output device found\n";
        return false;
    }

    // Stops and closes the stream if running
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
            // Close the stream; errors go through the callback
            m_audio->closeStream();
        }

        m_started = false;
        std::cout << "AudioEngine: Engine stopped\n";
    }

    // Configures and opens the output stream with the selected device
    bool AudioEngine::openAndStartStream(unsigned int deviceId)
    {
        RtAudio::StreamParameters outputParams;
        outputParams.deviceId = deviceId;
        outputParams.nChannels = m_settings.outputChannels;
        outputParams.firstChannel = 0;

        // Attempt to open the audio stream with a sine wave generator
        auto err = m_audio->openStream(
            &outputParams, nullptr, RTAUDIO_FLOAT32, m_settings.sampleRate,
            &m_settings.bufferFrames, &audioCallback, m_sine.get());

        if (err != RTAUDIO_NO_ERROR)
        {
            std::cerr << "AudioEngine: Failed to open stream: "
                      << m_audio->getErrorText() << "\n";
            return false;
        }

        // Start the stream
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

    // Returns true if a stream is open
    bool AudioEngine::isStreamOpen() const
    {
        return m_audio->isStreamOpen();
    }

    // Returns true if the stream is currently running
    bool AudioEngine::isStreamRunning() const
    {
        return m_audio->isStreamRunning();
    }

    // Delegates the current device info lookup to DeviceManager
    std::optional<RtAudio::DeviceInfo> AudioEngine::currentDeviceInfo() const
    {
        return m_deviceManager.currentDeviceInfo();
    }

} // namespace dtracker::engine
