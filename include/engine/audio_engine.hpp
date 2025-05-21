#pragma once
#include <optional>
#include <memory>
#include <RtAudio.h>

namespace dtracker::engine {
class AudioEngine {
public:
    AudioEngine();
    bool start();
    void stop();

    std::optional<unsigned int> getAvailableOutputDeviceId();


private:
    void loadSample();
    
    std::unique_ptr<RtAudio> m_audio;
};
}