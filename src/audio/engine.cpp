#include <dtracker/audio/engine.hpp>
#include <dtracker/audio/playback/tone_playback.hpp>
#include <dtracker/audio/types.hpp>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace dtracker::audio
{

    // RtAudio callback that fills the output buffer by rendering from the
    // current playback unit.
    // It runs on a high-priority, real-time audio thread.
    // The 'userData' is a raw pointer to the main playback unit (the proxy).
    int audioCallback(void *outputBuffer, void * /*inputBuffer*/,
                      unsigned int nFrames, double /*streamTime*/,
                      RtAudioStreamStatus status, void *userData)
    {
        if (status)
            std::cerr << "Stream underflow/overflow detected!\n";

        dtracker::audio::types::RenderContext context;

        float *buffer = static_cast<float *>(outputBuffer);
        constexpr unsigned int channels = 2;

        // Always clear the buffer to prevent leftover audio artifacts.
        std::memset(buffer, 0, sizeof(float) * nFrames * channels);

        // Only attempt rendering if a valid unit is provided
        if (auto *unit = static_cast<playback::ProxyPlaybackUnit *>(userData))
        {
            context.isLooping = unit->isLooping();
            unit->render(buffer, nFrames, channels, context);
        }
        else
        {
            std::cerr << "AudioEngine: No playback unit provided to callback\n";
        }

        return 0; // Continues stream
    }

    // Sets up RtAudio and the device manager
    Engine::Engine()
        : m_audio(std::make_unique<RtAudio>()),
          m_proxyUnit(std::make_unique<playback::ProxyPlaybackUnit>()),
          m_mixerUnit(std::make_unique<playback::MixerPlaybackUnit>())
    {
        std::cout << "AudioEngine: Initialized\n";

        // Route all audio through the proxy, which then delegates to the main
        // mixer. This decouples the C-style callback from the rest of the
        // system.
        m_proxyUnit->setDelegate(m_mixerUnit.get());
        m_audio->setErrorCallback(
            [](RtAudioErrorType type, const std::string &errorText)
            {
                std::cerr << "[RtAudio Error] (" << static_cast<int>(type)
                          << "): " << errorText << "\n";
            });
    }

    // Start the audio engine stream with the currently selected device
    bool Engine::start()
    {
        std::cout << "AudioEngine: Starting...\n";

        // A device must be selected before the engine can start.
        if (!m_selectedDeviceId.has_value())
        {
            std::cerr << "AudioEngine: No output device set\n";
            return false;
        }

        // openStream handles the actual interaction with RtAudio.
        const bool success = openStream(*m_selectedDeviceId);
        if (success)
            // Update state if successful.
            m_started = true;
        return success;
    }

    // Stop and close the active audio stream.
    void Engine::stop()
    {
        if (!m_started)
            return;

        // Stop the stream.
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

    // Internal helper that configures and opens the RtAudio stream.
    bool Engine::openStream(unsigned int deviceId)
    {
        RtAudio::StreamParameters outputParams;
        outputParams.deviceId = deviceId;
        outputParams.nChannels = m_settings.outputChannels;
        outputParams.firstChannel = 0;

        // Bind the C-style callback to our running engine instance by passing
        // a pointer to the proxy unit as the userdata argument.
        auto err = m_audio->openStream(
            &outputParams, nullptr, RTAUDIO_FLOAT32, m_settings.sampleRate,
            &m_settings.bufferFrames, &audioCallback, m_proxyUnit.get());

        if (err != RTAUDIO_NO_ERROR)
        {
            std::cerr << "AudioEngine: Failed to open stream: "
                      << m_audio->getErrorText() << "\n";
            return false;
        }

        // Start the stream.
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

    // Returns true if the audio stream is open.
    bool Engine::isStreamOpen() const
    {
        return m_audio->isStreamOpen();
    }

    // Returns true if the audio stream is currently running.
    bool Engine::isStreamRunning() const
    {
        return m_audio->isStreamRunning();
    }

    // Returns the current active output device info, if available.
    std::optional<unsigned int> Engine::currentDeviceId() const
    {
        return m_selectedDeviceId;
    }

    // Sets the selected output device.
    void Engine::setOutputDevice(unsigned int deviceId)
    {
        m_selectedDeviceId = deviceId;
    }

    // Creates a device manager with a reference to the internal RtAudio
    // instance.
    DeviceManager Engine::createDeviceManager() const
    {
        if (!m_audio)
            throw std::runtime_error(
                "Cannot create DeviceManager: m_audio is null");
        return DeviceManager(m_audio.get());
    }

    // Get a const pointer to the proxyPlaybackUnit.
    playback::ProxyPlaybackUnit *Engine::proxyPlaybackUnit()
    {
        return m_proxyUnit.get();
    }

    // Get access to the mixer unit.
    playback::MixerPlaybackUnit *Engine::mixerUnit()
    {
        return m_mixerUnit.get();
    }

    const types::AudioSettings &dtracker::audio::Engine::getSettings() const
    {
        // TODO: insert return statement here
        return m_settings;
    }

} // namespace dtracker::audio
