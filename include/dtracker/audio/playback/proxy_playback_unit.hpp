#pragma once
#include <atomic>
#include <dtracker/audio/playback/playback_unit.hpp>

namespace dtracker::audio::playback
{

    class ProxyPlaybackUnit : public PlaybackUnit
    {
      public:
        void render(float *buffer, unsigned int frames, unsigned int channels,
                    const types::RenderContext &context) override;

        void setDelegate(PlaybackUnit *unit);
        PlaybackUnit *delegate() const;

        void reset() override;
        bool isFinished() const;

        void setIsLooping(bool shouldLoop);
        bool isLooping() const;

      private:
        std::atomic<bool> m_isLooping{false};
        std::atomic<PlaybackUnit *> m_delegate{nullptr}; // Not owning
    };

} // namespace dtracker::audio::playback
