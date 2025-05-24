// proxy_playback_unit.hpp
#pragma once

#include "playback_unit.hpp"

namespace dtracker::audio::playback
{

    class ProxyPlaybackUnit : public PlaybackUnit
    {
      public:
        void render(float *buffer, unsigned int frames,
                    unsigned int channels) override;

        void setDelegate(PlaybackUnit *unit);
        PlaybackUnit *delegate() const;

        bool isFinished() const;

      private:
        PlaybackUnit *m_delegate{nullptr}; // Not owning
    };

} // namespace dtracker::audio::playback
