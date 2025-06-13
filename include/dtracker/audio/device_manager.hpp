#pragma once
#include <RtAudio.h>

#include <dtracker/audio/types.hpp>
#include <dtracker/audio/types/waves/sine_wav.hpp>
#include <memory>
#include <optional>

namespace dtracker::audio
{
    class DeviceManager
    {
      public:
        // funcs
        explicit DeviceManager(RtAudio *audio);

        std::optional<RtAudio::DeviceInfo> currentDeviceInfo() const;

      private:
        // funcs
        std::optional<unsigned int> findUsableOutputDevice();
        RtAudio::DeviceInfo getDeviceInfo(unsigned int id) const;

        // vars
        RtAudio *m_audio{nullptr};
        std::optional<RtAudio::DeviceInfo> m_currentDeviceInfo;
    };
} // namespace dtracker::audio