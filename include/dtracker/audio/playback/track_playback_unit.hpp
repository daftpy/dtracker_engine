#pragma once
#include <dtracker/audio/playback/playback_unit.hpp>
#include <memory>
#include <vector>

namespace dtracker::audio::playback
{

    class TrackPlaybackUnit : public PlaybackUnit
    {
      public:
        // void addSample(std::unique_ptr<PlaybackUnit> unit);
        void addSample(playback::PlaybackUnit *unit);
        void setVolume(float v);
        void setPan(float p); // -1.0 (left) to 1.0 (right)

        void render(float *buffer, unsigned int nFrames,
                    unsigned int channels) override;
        bool isFinished() const override;

      private:
        float m_volume = 1.0f;
        float m_pan = 0.0f; // -1.0 left, 0 center, 1.0 right
        // std::vector<std::unique_ptr<PlaybackUnit>> m_units;
        std::vector<playback::PlaybackUnit *> m_units;
    };
} // namespace dtracker::audio::playback