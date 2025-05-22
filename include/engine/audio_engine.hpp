#pragma once
#include <RtAudio.h>

#include "audio/waves/sine_wav.hpp"
#include "device_manager.hpp"
#include "types/audio_settings.hpp"
#include <memory>
#include <optional>

namespace dtracker::engine
{
    class AudioEngine
    {
      public:
        // funcs
        AudioEngine();

        bool start();
        void stop();

        std::optional<RtAudio::DeviceInfo> currentDeviceInfo() const;

        bool openAndStartStream(unsigned int deviceId);
        bool isStreamOpen() const;
        bool isStreamRunning() const;

        // vars
        bool m_started{false};

      private:
        // vars
        std::unique_ptr<RtAudio> m_audio;
        AudioSettings m_settings;

        DeviceManager m_deviceManager;
        std::unique_ptr<audio::waves::SineWave> m_sine;
    };
} // namespace dtracker::engine