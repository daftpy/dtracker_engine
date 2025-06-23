#pragma once

#include <dtracker/audio/device_manager.hpp>
#include <dtracker/audio/playback/mixer_playback.hpp>
#include <dtracker/audio/playback/proxy_playback_unit.hpp>
#include <optional>

namespace dtracker::audio
{
    /// Defines the public contract for the core audio engine.
    /// Its main responsibilities are managing the low-level audio stream and
    /// providing access to the master audio mixer for playback.
    class IEngine
    {
      public:
        /// Enables safe destruction of derived classes through a base pointer.
        virtual ~IEngine() = default;

        /// Starts the audio stream using the currently set output device.
        /// @return True on success, false if no device is set or stream fails.
        virtual bool start() = 0;

        /// Stops and closes the active audio stream.
        virtual void stop() = 0;

        /// Returns true if the audio stream is currently active and processing.
        virtual bool isStreamRunning() const = 0;

        /// Sets the hardware device to be used for audio output.
        virtual void setOutputDevice(unsigned int deviceId) = 0;

        /// Gets a raw pointer to the main mixer unit.
        /// Clients use this to add playable audio sources to the engine.
        virtual playback::MixerPlaybackUnit *mixerUnit() = 0;

        virtual playback::ProxyPlaybackUnit *proxyUnit() = 0;

        /// Creates a helper object for querying available audio devices.
        virtual DeviceManager createDeviceManager() const = 0;

        virtual const types::AudioSettings &getSettings() const = 0;
    };

} // namespace dtracker::audio