#pragma once
#include <RtAudio.h>

#include "audio/waves/sine_wav.hpp"
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
        bool isStreamOpen() const;
        bool isStreamRunning() const;

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

        std::unique_ptr<audio::waves::SineWave> m_sine;
    };
} // namespace dtracker::engine