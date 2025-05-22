#pragma once
#include <RtAudio.h>

#include "audio/types/audio_settings.hpp"
#include "audio/types/waves/sine_wav.hpp"
#include "device_manager.hpp"
#include <memory>
#include <optional>

namespace dtracker::audio
{
    class Engine
    {
      public:
        // funcs
        Engine();

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
        audio::types::AudioSettings m_settings;

        DeviceManager m_deviceManager;
        std::unique_ptr<audio::types::waves::SineWave> m_sine;
    };
} // namespace dtracker::audio