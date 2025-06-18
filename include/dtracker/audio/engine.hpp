#pragma once

#include <RtAudio.h>

#include "i_engine.hpp"
#include <dtracker/audio/device_manager.hpp>
#include <dtracker/audio/playback/mixer_playback.hpp>
#include <dtracker/audio/playback/proxy_playback_unit.hpp>
#include <dtracker/audio/types.hpp>
#include <memory>
#include <optional>

namespace dtracker::audio
{
    /// An implementation of the IEngine interface using the RtAudio
    /// library.
    class Engine : public IEngine
    {
      public:
        Engine();
        ~Engine() = default;

        // --- IEngine Interface Implementation ---
        // See i_engine.hpp for detailed documentation on these methods.
        bool start() override;
        void stop() override;
        bool isStreamRunning() const override;
        void setOutputDevice(unsigned int deviceId) override;
        playback::MixerPlaybackUnit *mixerUnit() override;
        DeviceManager createDeviceManager() const override;
        types::AudioSettings const &getSettings() const override;

        // --- Engine-Specific Public Methods ---
        // These provide lower-level access not part of the general IEngine
        // contract.
        std::optional<unsigned int> currentDeviceId() const;
        bool isStreamOpen() const;

      private:
        // Helper to bridge the engine with RtAudio's C-style callback.
        playback::ProxyPlaybackUnit *proxyPlaybackUnit();

        // Helper to start and open the stream
        bool openStream(unsigned int deviceId);

        std::unique_ptr<RtAudio> m_audio;               // RtAudio instance
        audio::types::AudioSettings m_settings;         // Audio stream settings
        std::optional<unsigned int> m_selectedDeviceId; // Selected device ID

        // The root of our audio graph.
        std::unique_ptr<playback::MixerPlaybackUnit> m_mixerUnit;
        // The proxy that is passed to the RtAudio callback.
        std::unique_ptr<playback::ProxyPlaybackUnit> m_proxyUnit;

        // Internal state flag.
        bool m_started{false};
    };
} // namespace dtracker::audio