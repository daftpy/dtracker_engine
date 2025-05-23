#include "audio/engine.hpp"

#include "audio/playback/tone_playback.hpp"
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace dtracker::audio
{

    // RtAudio callback that fills the output buffer by rendering from the
    // current playback unit
    int audioCallback(void *outputBuffer, void * /*inputBuffer*/,
                      unsigned int nFrames, double /*streamTime*/,
                      RtAudioStreamStatus status, void *userData)
    {
        if (status)
            std::cerr << "Stream underflow/overflow detected!\n";

        auto *unit = static_cast<playback::PlaybackUnit *>(userData);
        float *buffer = static_cast<float *>(outputBuffer);

        constexpr unsigned int channels = 2; // Hardcoded stereo output
        std::memset(buffer, 0,
                    sizeof(float) * nFrames * channels); // Clear the buffer

        unit->render(buffer, nFrames, channels); // Render audio into the buffer

        return 0;
    }

    // Sets up RtAudio and the device manager
    Engine::Engine()
        : m_audio(std::make_unique<RtAudio>()), m_deviceManager(m_audio.get())
    {
        std::cout << "AudioEngine: Initialized\n";

        // Register error callback with RtAudio
        m_audio->setErrorCallback(
            [](RtAudioErrorType type, const std::string &errorText)
            {
                std::cerr << "[RtAudio Error] (" << static_cast<int>(type)
                          << "): " << errorText << "\n";
            });
    }

    // Start the audio engine and playback stream
    bool Engine::start()
    {
        std::cout << "AudioEngine: Starting...\n";

        auto currentDeviceInfo = m_deviceManager.currentDeviceInfo();
        if (currentDeviceInfo.has_value())
        {
            auto &info = currentDeviceInfo.value();
            m_started = true;

            std::cout << "Using device: " << info.name << " (" << info.ID
                      << ")\n";
            std::cout << "Output channels: " << info.outputChannels << "\n";

            // Set up playback unit (sine tone for now)
            m_currentPlayback =
                std::make_unique<playback::TonePlayback>(static_cast<float>(
                    m_settings.sampleRate)); // Cast sampleRate to float for DSP
                                             // calculations

            return openAndStartStream(info.ID);
        }

        std::cerr << "AudioEngine: No valid output device found\n";
        return false;
    }

    // Stop and close the active audio stream
    void Engine::stop()
    {
        if (!m_started)
            return;

        // Stop the stream
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

        // Close the stream
        if (m_audio->isStreamOpen())
        {
            m_audio
                ->closeStream(); // No return value; error goes through callback
        }

        m_started = false;
        std::cout << "AudioEngine: Engine stopped\n";
    }

    // Opens and starts the audio stream with the provided output device ID
    bool Engine::openAndStartStream(unsigned int deviceId)
    {
        RtAudio::StreamParameters outputParams;
        outputParams.deviceId = deviceId;
        outputParams.nChannels = m_settings.outputChannels;
        outputParams.firstChannel = 0;

        // Bind playback unit to callback and open thee stream
        auto err = m_audio->openStream(
            &outputParams, nullptr, RTAUDIO_FLOAT32, m_settings.sampleRate,
            &m_settings.bufferFrames, &audioCallback, m_currentPlayback.get());

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

    // Returns true if the audio stream is open
    bool Engine::isStreamOpen() const
    {
        return m_audio->isStreamOpen();
    }

    // Returns true if the audio stream is currently running
    bool Engine::isStreamRunning() const
    {
        return m_audio->isStreamRunning();
    }

    // Returns the current active output device info, if available
    std::optional<RtAudio::DeviceInfo> Engine::currentDeviceInfo() const
    {
        return m_deviceManager.currentDeviceInfo();
    }

} // namespace dtracker::audio
