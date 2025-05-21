#pragma once
#include <RtAudio.h>

#include <memory>
#include <optional>

namespace dtracker::engine
{
    class AudioEngine
    {
      public:
        AudioEngine();
        bool start();
        void stop();

      private:
        // funcs
        std::optional<unsigned int> findUsableOutputDevice();
        RtAudio::DeviceInfo getDeviceInfo(unsigned int id) const;

        // vars
        std::unique_ptr<RtAudio> m_audio;
    };
} // namespace dtracker::engine