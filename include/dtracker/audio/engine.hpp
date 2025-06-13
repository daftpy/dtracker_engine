#pragma once
#include <RtAudio.h>

#include <dtracker/audio/device_manager.hpp>
#include <dtracker/audio/playback/mixer_playback.hpp>
#include <dtracker/audio/playback/playback_unit.hpp>
#include <dtracker/audio/playback/proxy_playback_unit.hpp>
#include <dtracker/audio/types.hpp>
#include <dtracker/audio/types/waves/sine_wav.hpp>
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
        playback::ProxyPlaybackUnit *proxyPlaybackUnit();
        playback::MixerPlaybackUnit *mixerUnit();

        DeviceManager createDeviceManager() const;

        // vars
        bool m_started{false};

      private:
        // vars
        std::unique_ptr<RtAudio> m_audio;
        audio::types::AudioSettings m_settings;
        std::optional<unsigned int> m_selectedDeviceId;

        std::unique_ptr<playback::MixerPlaybackUnit> m_mixerUnit;
        std::unique_ptr<playback::ProxyPlaybackUnit> m_proxyUnit;
    };
} // namespace dtracker::audio