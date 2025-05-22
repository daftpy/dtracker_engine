#pragma once
#include <RtAudio.h>

#include "audio_settings.hpp"
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

        RtAudio::DeviceInfo currentDeviceInfo() const;

        bool openAndStartStream(unsigned int deviceId);

        // vars
        bool m_started{false};

      private:
        // funcs
        std::optional<unsigned int> findUsableOutputDevice();
        RtAudio::DeviceInfo getDeviceInfo(unsigned int id) const;

        // vars
        std::unique_ptr<RtAudio> m_audio;
        AudioSettings m_settings;
        RtAudio::DeviceInfo m_currentDeviceInfo;
    };
} // namespace dtracker::engine