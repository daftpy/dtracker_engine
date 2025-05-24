#pragma once
#include <RtAudio.h>

#include "device_manager.hpp"
#include "playback/playback_unit.hpp"
#include "playback/proxy_playback_unit.hpp"
#include "types/audio_settings.hpp"
#include "types/waves/sine_wav.hpp"
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

        std::optional<unsigned int> currentDeviceId() const;

        bool openStream(unsigned int deviceId);
        bool isStreamOpen() const;
        bool isStreamRunning() const;

        void setOutputDevice(unsigned int deviceId);
        void setPlaybackUnit(std::unique_ptr<playback::PlaybackUnit> unit);
        playback::ProxyPlaybackUnit *proxyPlaybackUnit() const;

        DeviceManager createDeviceManager() const;

        // vars
        bool m_started{false};

      private:
        // vars
        std::unique_ptr<RtAudio> m_audio;
        audio::types::AudioSettings m_settings;
        std::optional<unsigned int> m_selectedDeviceId;

        std::unique_ptr<playback::PlaybackUnit> m_currentPlayback;
        std::unique_ptr<playback::ProxyPlaybackUnit> m_proxyUnit;
    };
} // namespace dtracker::audio