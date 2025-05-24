#pragma once

#include <dtracker/audio/playback/playback_unit.hpp>

class MockPlaybackUnit : public dtracker::audio::playback::PlaybackUnit
{
  public:
    int renderCallCount = 0;

    void render(float *buffer, unsigned int frames,
                unsigned int channels) override
    {
        renderCallCount++;
    }

    bool isFinished() const override
    {
        return false;
    }
};
